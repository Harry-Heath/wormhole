const std = @import("std");

pub fn build(b: *std.Build) void {
    const target = b.standardTargetOptions(.{});
    const optimize = b.standardOptimizeOption(.{});

    const mod = b.createModule(.{
        .target = target,
        .optimize = optimize,
        .link_libc = true,
        .link_libcpp = true,
    });

    mod.addCSourceFile(.{
        .file = b.path("src/main.cpp"),
        .flags = &.{"-std=c++20"},
    });

    const exe = b.addExecutable(.{
        .name = "statey",
        .root_module = mod,
    });

    b.installArtifact(exe);

    const run_step = b.step("run", "Runs");
    run_step.dependOn(&b.addRunArtifact(exe).step);

    //
    const mod2 = b.createModule(.{
        .target = target,
        .optimize = optimize,
        .root_source_file = b.path("src/main.zig"),
    });
    const exe2 = b.addExecutable(.{
        .name = "statey2",
        .root_module = mod2,
    });
    b.installArtifact(exe2);

    const run_step2 = b.step("run2", "Runs2");
    const run2 = b.addRunArtifact(exe2);
    if (b.args) |args| {
        run2.addArgs(args);
    }
    run_step2.dependOn(&run2.step);
}
