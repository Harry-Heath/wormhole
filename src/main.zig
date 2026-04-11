const std = @import("std");
const zon = std.zon.parse;
const Io = std.Io;
const Dir = Io.Dir;

const Schema = @import("Schema.zig");
const cli = @import("cli.zig");
const cpp = @import("cpp.zig");
const ts = @import("ts.zig");

pub fn main(init: std.process.Init) !void {
    const gpa = init.arena.allocator();
    const io = init.io;
    const args = try init.minimal.args.toSlice(gpa);
    const cwd = Dir.cwd();

    // Parse args
    const parsed_args = cli.parseArgs(args) catch |err| {
        try cli.printHelp(io);
        switch (err) {
            error.Help => return,
            error.MissingArgs, error.BadArgs => return err,
        }
    };

    // Read file
    const file = try cwd.readFileAllocOptions(io, parsed_args.input, gpa, .unlimited, .of(u8), 0);

    // Parse zon
    var diag: zon.Diagnostics = .{};
    const schema = zon.fromSliceAlloc(Schema, gpa, file, &diag, .{}) catch |err| switch (err) {
        error.ParseZon => {
            const stderr = try io.lockStderr(&.{}, null);
            defer io.unlockStderr();

            const writer = &stderr.file_writer.interface;
            try writer.print("Failed to parse zon file:\n", .{});
            try diag.format(writer);
            return err;
        },
        else => return err,
    };

    // Generate file
    const output = switch (parsed_args.language) {
        .cpp => try cpp.write(.{
            .schema = schema,
            .gpa = gpa,
            .embed = parsed_args.embed_library,
        }),
        .ts => try ts.write(.{
            .schema = schema,
            .gpa = gpa,
            .embed = parsed_args.embed_library,
        }),
    };

    // Write to file
    try cwd.writeFile(io, .{
        .sub_path = parsed_args.output,
        .data = output,
    });
}
