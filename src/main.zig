const std = @import("std");
const zon = std.zon.parse;
const Io = std.Io;
const Dir = Io.Dir;

pub fn main(init: std.process.Init) !void {
    const gpa = init.arena.allocator();
    const io = init.io;
    const args = try init.minimal.args.toSlice(gpa);
    const cwd = Dir.cwd();

    // Read file
    const filename = args[1];
    const file = try cwd.readFileAllocOptions(io, filename, gpa, .unlimited, .of(u8), 0);

    // Parse zon
    var diag: zon.Diagnostics = .{};
    const parsed = zon.fromSliceAlloc(Schema, gpa, file, &diag, .{}) catch |err| switch (err) {
        error.ParseZon => {
            const stdout = try io.lockStderr(&.{}, null);
            defer io.unlockStderr();

            const writer = &stdout.file_writer.interface;
            try writer.print("Failed to parse zon file:\n", .{});
            try diag.format(writer);
            std.process.exit(1);
        },
        else => return err,
    };

    std.debug.print("{?s}", .{parsed.types[0].description});
}

const Schema = struct {
    types: []const Type = &.{},
    properties: []const Property,
};

const Type = struct {
    name: []const u8,
    description: ?[]const u8 = null,
    fields: ?[]const Field = null,
    values: ?[]const EnumValue = null,
};

const Property = struct {
    name: []const u8,
    description: ?[]const u8 = null,
    id: u8,
    array: ?Array = null,
    type: ?[]const u8 = null,
    properties: ?[]const Property = null,
};

const Field = struct {
    name: []const u8,
    description: ?[]const u8 = null,
    array: ?Array = null,
    type: ?[]const u8 = null,
    fields: ?[]const Field = null,
};

const EnumValue = struct {
    name: []const u8,
    value: u8,
};

const Array = union(enum) {
    variable_length,
    length: u8,
};
