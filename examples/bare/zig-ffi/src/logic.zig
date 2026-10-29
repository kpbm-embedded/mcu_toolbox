//! Freestanding Zig called from C (and calling back into C): CRC-16/CCITT, a bit-reversal and a comptime table.
extern fn bare_puts(s: [*:0]const u8) void;

const table = blk: {
    @setEvalBranchQuota(10000);
    var t: [256]u16 = undefined;
    for (&t, 0..) |*e, i| {
        var c: u16 = @as(u16, @intCast(i)) << 8;
        for (0..8) |_| c = if (c & 0x8000 != 0) (c << 1) ^ 0x1021 else c << 1;
        e.* = c;
    }
    break :blk t; // computed at compile time, stored in flash
};

export fn zig_crc16_ccitt(data: [*]const u8, len: usize) u16 {
    var crc: u16 = 0xFFFF;
    for (data[0..len]) |b| crc = (crc << 8) ^ table[@as(u8, @truncate(crc >> 8)) ^ b];
    return crc;
}

export fn zig_bitrev32(x: u32) u32 {
    return @bitReverse(x);
}

export fn zig_hello() void {
    bare_puts("hello from Zig (freestanding)\n");
}
