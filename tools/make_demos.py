#!/usr/bin/env python3
"""Build the original demo disks shipped with RETRO Center.

Nothing here is a commercial game. Drift is a small C64 BASIC program,
the Apple II image is an original boot sector, and the Atari ST file is
an original GEMDOS program.
"""

import pathlib
import struct

ROOT = pathlib.Path(__file__).resolve().parents[1]
OUT = ROOT / "data" / "demos"


# Commodore 64 BASIC V2 tokens, longest match first.
TOKENS = [
    ("INPUT#", 0x84),
    ("PRINT#", 0x98),
    ("RIGHT$", 0xC9),
    ("LEFT$", 0xC8),
    ("RESTORE", 0x8C),
    ("RETURN", 0x8E),
    ("VERIFY", 0x95),
    ("CHR$", 0xC7),
    ("MID$", 0xCA),
    ("STR$", 0xC4),
    ("GOTO", 0x89),
    ("GOSUB", 0x8D),
    ("PRINT", 0x99),
    ("INPUT", 0x85),
    ("CLOSE", 0xA0),
    ("STEP", 0xA9),
    ("THEN", 0xA7),
    ("STOP", 0x90),
    ("CONT", 0x9A),
    ("READ", 0x87),
    ("NEXT", 0x82),
    ("DATA", 0x83),
    ("POKE", 0x97),
    ("OPEN", 0x9F),
    ("LOAD", 0x93),
    ("SAVE", 0x94),
    ("WAIT", 0x92),
    ("PEEK", 0xC2),
    ("SPC(", 0xA6),
    ("TAB(", 0xA3),
    ("SGN", 0xB4),
    ("INT", 0xB5),
    ("ABS", 0xB6),
    ("USR", 0xB7),
    ("FRE", 0xB8),
    ("POS", 0xB9),
    ("SQR", 0xBA),
    ("RND", 0xBB),
    ("LOG", 0xBC),
    ("EXP", 0xBD),
    ("COS", 0xBE),
    ("SIN", 0xBF),
    ("TAN", 0xC0),
    ("ATN", 0xC1),
    ("LEN", 0xC3),
    ("VAL", 0xC5),
    ("ASC", 0xC6),
    ("AND", 0xAF),
    ("FOR", 0x81),
    ("END", 0x80),
    ("DIM", 0x86),
    ("LET", 0x88),
    ("RUN", 0x8A),
    ("IF", 0x8B),
    ("REM", 0x8F),
    ("ON", 0x91),
    ("DEF", 0x96),
    ("CLR", 0x9C),
    ("CMD", 0x9D),
    ("SYS", 0x9E),
    ("GET", 0xA1),
    ("NEW", 0xA2),
    ("FN", 0xA5),
    ("NOT", 0xA8),
    ("TO", 0xA4),
    ("OR", 0xB0),
    ("GO", 0xCB),
    (">", 0xB1),
    ("=", 0xB2),
    ("<", 0xB3),
    ("+", 0xAA),
    ("-", 0xAB),
    ("*", 0xAC),
    ("/", 0xAD),
    ("^", 0xAE),
]
BY_CODE = {code: name for name, code in TOKENS}

DRIFT = """
10 POKE 53280,0:POKE 53281,0:PRINT CHR$(147)
20 S=0:P=20:OP=20
30 FOR I=1 TO 3:X(I)=INT(RND(1)*30)+2:Y(I)=INT(RND(1)*8):NEXT
40 PRINT CHR$(19);"DRIFT   A LEFT   D RIGHT   SCORE ";S
50 POKE 1024+22*40+OP,32
60 GET A$:IF A$="A" THEN P=P-1
70 IF A$="D" THEN P=P+1
80 IF P<1 THEN P=1
90 IF P>38 THEN P=38
100 FOR I=1 TO 3
110 POKE 1024+Y(I)*40+X(I),32
120 Y(I)=Y(I)+1
130 IF Y(I)>22 THEN Y(I)=0:X(I)=INT(RND(1)*30)+2:S=S+1:PRINT CHR$(19);"DRIFT   A LEFT   D RIGHT   SCORE ";S
140 IF Y(I)=22 AND X(I)=P THEN GOTO 210
150 POKE 1024+Y(I)*40+X(I),81
160 POKE 55296+Y(I)*40+X(I),10
170 NEXT
180 POKE 1024+22*40+P,90
190 POKE 55296+22*40+P,1
200 OP=P:GOTO 50
210 PRINT CHR$(147);"CRASH    SCORE ";S
220 PRINT "PRESS SPACE"
230 GET A$:IF A$<>" " THEN GOTO 230
240 GOTO 10
""".strip().splitlines()


def tokenize(text):
    out = bytearray()
    i = 0
    while i < len(text):
        if text[i] == '"':
            j = i + 1
            while j < len(text) and text[j] != '"':
                j += 1
            j = min(j + 1, len(text))
            out += text[i:j].encode("latin1")
            i = j
            continue
        matched = False
        upper = text[i:].upper()
        for name, code in TOKENS:
            if upper.startswith(name):
                out.append(code)
                i += len(name)
                matched = True
                break
        if matched:
            continue
        out.append(ord(text[i]))
        i += 1
    return out


def detokenize(tok):
    chars = []
    for byte in tok:
        if byte in BY_CODE:
            chars.append(BY_CODE[byte])
        else:
            chars.append(chr(byte))
    return "".join(chars)


def encode_basic(lines):
    parsed = []
    for raw in lines:
        raw = raw.strip()
        if not raw:
            continue
        num, _, rest = raw.partition(" ")
        tok = tokenize(rest)
        back = detokenize(tok)
        if back != rest:
            raise SystemExit(f"token round-trip failed\n in: {rest}\nout: {back}")
        parsed.append((int(num), tok))

    chunks = []
    for num, tok in parsed:
        chunks.append(bytearray([0, 0, num & 255, num >> 8]) + tok + bytes([0]))

    addr = 0x0801
    for chunk in chunks:
        addr += len(chunk)
        chunk[0] = addr & 255
        chunk[1] = (addr >> 8) & 255
    body = b"".join(chunks) + bytes([0, 0])
    return bytes([0x01, 0x08]) + body


class Asm6502:
    def __init__(self, org):
        self.org = org
        self.here = org
        self.out = bytearray()
        self.labels = {}
        self.fixups = []

    def label(self, name):
        self.labels[name] = self.here

    def emit(self, *raw):
        self.out.extend(raw)
        self.here += len(raw)

    def abs(self, name):
        self.fixups.append((len(self.out), name, "abs"))
        self.emit(0, 0)

    def rel(self, name):
        self.fixups.append((len(self.out), name, "rel"))
        self.emit(0)

    def finish(self):
        for pos, name, kind in self.fixups:
            target = self.labels[name]
            if kind == "abs":
                self.out[pos] = target & 0xFF
                self.out[pos + 1] = (target >> 8) & 0xFF
            else:
                src = self.org + pos + 1
                rel = target - src
                if not -128 <= rel <= 127:
                    raise SystemExit(f"branch to {name} is out of range ({rel})")
                self.out[pos] = rel & 0xFF
        return bytes(self.out)


def apple_text(text):
    raw = []
    for ch in text:
        if ch == " ":
            raw.append(0xA0)
        elif "A" <= ch <= "Z":
            raw.append(0xC1 + ord(ch) - ord("A"))
        elif "0" <= ch <= "9":
            raw.append(0xB0 + ord(ch) - ord("0"))
        else:
            raw.append(0xA0)
    raw.append(0)
    return raw


def build_apple_disk():
    # Boot ROM loads track 0 sector 0 to $800 and jumps to $801.
    # The byte at $800 is unused by the ROM; 1 matches a one-sector boot.
    code = Asm6502(0x800)
    code.emit(0x01)
    code.label("start")
    for switch in (0xC051, 0xC052, 0xC054):  # text, full, page 1
        code.emit(0x2C)  # BIT abs
        code.emit(switch & 0xFF, switch >> 8)
    code.emit(0xA2, 0x00)  # LDX #0
    code.label("clr")
    code.emit(0xA9, 0xA0)  # LDA #$A0
    for page in (0x0400, 0x0480, 0x0500, 0x0580, 0x0600, 0x0680, 0x0700, 0x0780):
        code.emit(0x9D, page & 0xFF, page >> 8)  # STA page,X
    code.emit(0xE8)  # INX
    code.emit(0xD0)
    code.rel("clr")

    def draw(label, dest, text):
        code.emit(0xA2, 0x00)
        code.label(label)
        code.emit(0xBD)
        code.abs(label + "_m")
        code.emit(0xF0)
        code.rel(label + "_done")
        code.emit(0x9D, dest & 0xFF, dest >> 8)
        code.emit(0xE8)
        code.emit(0xD0)
        code.rel(label)
        code.label(label + "_done")

    # Row 9 ($04A8) and row 11 ($05A8), centered.
    line1 = "RETRO CENTER"
    line2 = "APPLE II IS ON"
    draw("d1", 0x04A8 + (40 - len(line1)) // 2, line1)
    draw("d2", 0x05A8 + (40 - len(line2)) // 2, line2)

    code.label("spin")
    code.emit(0xA9, 0xAA)  # '*'
    code.emit(0x8D, 0xE3, 0x07)  # STA $07E3, middle of the last row
    code.emit(0x20)
    code.abs("delay")
    code.emit(0xA9, 0xA0)
    code.emit(0x8D, 0xE3, 0x07)
    code.emit(0x20)
    code.abs("delay")
    code.emit(0x4C)
    code.abs("spin")

    code.label("delay")
    code.emit(0xA0, 0x18)  # LDY #$18
    code.label("d1")
    code.emit(0xA2, 0xFF)  # LDX #$FF
    code.label("d2")
    code.emit(0xCA)  # DEX
    code.emit(0xD0)
    code.rel("d2")
    code.emit(0x88)  # DEY
    code.emit(0xD0)
    code.rel("d1")
    code.emit(0x60)  # RTS

    code.label("d1_m")
    code.emit(*apple_text(line1))
    code.label("d2_m")
    code.emit(*apple_text(line2))

    program = code.finish()
    if len(program) > 256:
        raise SystemExit(f"Apple boot sector is {len(program)} bytes")
    disk = bytearray(35 * 16 * 256)
    disk[: len(program)] = program
    return bytes(disk)


def build_st_prg():
    # PC-relative GEMDOS hello. ph_absflag is set, so no relocation table.
    message = (
        b"RETRO CENTER\r\n"
        b"Atari ST is on.\r\n"
        b"Press a key to leave.\r\n\x00"
    )
    # lea msg(pc),a0 / print / wait / exit. Displacement filled below.
    text = bytearray(
        [
            0x41, 0xFA, 0x00, 0x00,  # lea msg(pc),a0
            0x2F, 0x08,  # move.l a0,-(sp)
            0x3F, 0x3C, 0x00, 0x09,  # move.w #Cconws,-(sp)
            0x4E, 0x41,  # trap #1
            0x5C, 0x8F,  # addq.l #6,sp
            0x3F, 0x3C, 0x00, 0x01,  # move.w #Cconin,-(sp)
            0x4E, 0x41,
            0x54, 0x8F,  # addq.l #2,sp
            0x42, 0x67,  # clr.w -(sp)  Pterm0
            0x4E, 0x41,
        ]
    )
    disp = len(text) - 2  # msg offset minus the PC of the displacement word
    if not 0 <= disp <= 0x7FFF:
        raise SystemExit("ST displacement out of range")
    text[2] = (disp >> 8) & 0xFF
    text[3] = disp & 0xFF
    text += message
    if len(text) % 2:
        text.append(0)
    header = struct.pack(
        ">HIIIIIIH",
        0x601A,  # ph_branch
        len(text),
        0,  # data
        0,  # bss
        0,  # symbols
        0,  # reserved
        0,  # flags
        1,  # absolute, no relocation
    )
    return header + bytes(text)


def main():
    OUT.mkdir(parents=True, exist_ok=True)
    prg = encode_basic(DRIFT)
    (OUT / "drift.prg").write_bytes(prg)
    disk = build_apple_disk()
    (OUT / "retro-center.dsk").write_bytes(disk)
    tos = build_st_prg()
    (OUT / "retro-center.prg").write_bytes(tos)
    print(f"drift.prg          {len(prg)} bytes")
    print(f"retro-center.dsk   {len(disk)} bytes")
    print(f"retro-center.prg   {len(tos)} bytes")


if __name__ == "__main__":
    main()
