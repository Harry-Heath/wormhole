const std = @import("std");
const zon = std.zon.parse;
const Io = std.Io;
const Dir = Io.Dir;
const Schema = @import("Schema.zig");

const cpp = @import("cpp.zig");
const ts = @import("ts.zig");

pub fn main(init: std.process.Init) !void {
    const gpa = init.arena.allocator();
    const io = init.io;
    const args = try init.minimal.args.toSlice(gpa);
    const cwd = Dir.cwd();

    if (args.len != 4)
        return error.BadArgCount;

    // Args
    const input_filename = args[1];
    const lang = args[2];
    const output_filename = args[3];

    // Read file
    const file = try cwd.readFileAllocOptions(io, input_filename, gpa, .unlimited, .of(u8), 0);

    // Parse zon
    var diag: zon.Diagnostics = .{};
    const schema = zon.fromSliceAlloc(Schema, gpa, file, &diag, .{}) catch |err| switch (err) {
        error.ParseZon => {
            const stderr = try io.lockStderr(&.{}, null);
            defer io.unlockStderr();

            const writer = &stderr.file_writer.interface;
            try writer.print("Failed to parse zon file:\n", .{});
            try diag.format(writer);
            std.process.exit(1);
        },
        else => return err,
    };

    // Generate file
    var output: []const u8 = &.{};
    if (std.mem.eql(u8, lang, "cpp")) {
        output = try cpp.write(schema, gpa);
    } else if (std.mem.eql(u8, lang, "ts")) {
        output = try ts.write(schema, gpa);
    } else return error.UnknownLang;

    // Write to file
    try cwd.writeFile(io, .{
        .sub_path = output_filename,
        .data = output,
    });
}
