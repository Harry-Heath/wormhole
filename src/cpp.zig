const std = @import("std");
const Schema = @import("Schema.zig");
const IndentedWriter = @import("IndentedWriter.zig");
const case = @import("case");
const Self = @This();

const Map = std.StringHashMap([]const u8);

schema: Schema,
gpa: std.mem.Allocator,
writer: *IndentedWriter,
type_map: Map,

pub fn write(schema: Schema, gpa: std.mem.Allocator) ![]const u8 {
    var string: std.Io.Writer.Allocating = .init(gpa);
    var writer: IndentedWriter = .init(&string.writer);

    var self: Self = .{
        .schema = schema,
        .gpa = gpa,
        .writer = &writer,
        .type_map = .init(gpa),
    };

    try self.setupTypeMap();
    try self.writeTypes();
    try self.writeProperties();

    return string.written();
}

fn setupTypeMap(self: *Self) !void {
    const map = &self.type_map;

    try map.put("u8", "uint8_t");
    try map.put("u16", "uint16_t");
    try map.put("u32", "uint32_t");
    try map.put("u64", "uint64_t");

    try map.put("s8", "int8_t");
    try map.put("s16", "int16_t");
    try map.put("s32", "int32_t");
    try map.put("s64", "int64_t");

    try map.put("f32", "float");
    try map.put("f64", "double");

    for (self.schema.types) |t| {
        try map.put(t.name, try case.allocTo(self.gpa, .pascal, t.name));
    }
}

fn writeTypes(self: *Self) !void {
    for (self.schema.types) |t| {
        try self.writeType(t);
    }
}

fn writeType(self: *Self, t: Schema.Type) !void {
    const writer = self.writer;

    // Must have either fields or values
    if ((t.fields != null) == (t.values != null)) {
        // TODO: print error
        return error.BadZon;
    }

    // Write struct
    if (t.fields) |fields| {
        try writer.print("struct {s}", .{self.type_map.get(t.name).?});
        try writer.print("{{", .{});
        writer.indent();

        for (fields) |field| {
            const field_type = self.type_map.get(field.type) orelse {
                // TODO: print error
                return error.BadZon;
            };
            const field_name = try case.allocTo(self.gpa, .snake, field.name);

            // Add array field
            if (field.array) |array| switch (array) {
                .variable_length => try writer.print("std::vector<{s}> {s};", .{ field_type, field_name }),
                .length => |length| try writer.print("{s} {s}[{}];", .{ field_type, field_name, length }),
            }
            // Otherwise just type
            else try writer.print("{s} {s};", .{ field_type, field_name });
        }

        writer.deindent();
        try writer.print("}};", .{});
    }

    // Write enum
    if (t.values) |values| {
        try writer.print("enum class {s}", .{self.type_map.get(t.name).?});
        try writer.print("{{", .{});
        writer.indent();

        for (values) |value| {
            const value_name = try case.allocTo(self.gpa, .constant, value.name);
            try writer.print("{s} = {},", .{ value_name, value.value });
        }

        writer.deindent();
        try writer.print("}};", .{});
    }
}

fn writeStruct(self: *Self, s: Schema.Struct) !void {
    const writer = self.writer;

    try writer.print("struct {s}", .{});
    try writer.print("{{", .{});
    writer.indent();

    for (self.schema.properties) |property| {
        try self.writeProperty(property);
    }

    writer.deindent();
    try writer.print("}};", .{});
}

fn writeProperty(self: *Self, property: Schema.Property) !void {
    const writer = self.writer;

    // Must have either type or properties
    if ((property.type != null) == (property.properties != null)) {
        // TODO: print error
        return error.BadZon;
    }

    _ = writer;
}
