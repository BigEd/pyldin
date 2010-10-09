program formater;
uses dos, crt;

type
  byte_arr = array[0..$7FFF] of byte;
  byte_ptr = ^byte_arr;
  boot_arr = array[0..511] of byte;

const
  max_retry = 3;

  boot_init: boot_arr = (
$EB,$4B,$90,$55,$6E,$69,$44,$4F,$53,$20,$20,$00,$02,$02,$01,$00,
$02,$70,$00,$D0,$02,$FD,$02,$00,$09,$00,$02,$00,$00,$00,$00,$00,
$00,$00,$00,$00,$00,$00,$00,$00,$00,$00,$00,$00,$00,$00,$00,$12,
$00,$00,$00,$00,$01,$00,$00,$00,$00,$00,$00,$00,$00,$00,$00,$00,
$00,$00,$00,$00,$00,$00,$00,$00,$00,$00,$00,$00,$00,$E9,$B0,$00,
$BD,$01,$B3,$FF,$01,$AA,$CE,$00,$00,$FF,$01,$AC,$86,$01,$CE,$01,
$AA,$3F,$4A,$4D,$26,$32,$F7,$01,$B2,$17,$CE,$01,$AE,$3F,$51,$FE,
$01,$AE,$26,$24,$FE,$01,$B0,$FF,$01,$AC,$CE,$10,$00,$FF,$01,$AA,
$B6,$01,$B2,$CE,$01,$AA,$3F,$4C,$4D,$26,$0D,$BC,$01,$B0,$26,$08,
$B6,$01,$B2,$3F,$4E,$7E,$10,$00,$CE,$02,$60,$3F,$23,$3F,$20,$7F,
$E6,$C0,$FE,$FF,$FE,$01,$01,$01,$6E,$00,$00,$00,$00,$00,$00,$00,
$00,$00,$00,$CE,$00,$00,$3F,$41,$4D,$26,$07,$C1,$02,$26,$03,$7F,
$EB,$BD,$CE,$02,$AA,$39,$00,$00,$00,$00,$00,$00,$00,$00,$00,$00,
$00,$00,$00,$00,$00,$00,$00,$00,$00,$00,$00,$00,$00,$00,$00,$00,
$00,$00,$00,$00,$00,$00,$00,$00,$00,$00,$00,$00,$00,$00,$00,$00,
$00,$00,$00,$00,$00,$00,$00,$00,$00,$00,$00,$00,$00,$00,$00,$00,
$0E,$1F,$BE,$60,$7D,$FC,$AC,$0A,$C0,$74,$06,$B4,$0E,$CD,$10,$EB,
$F5,$33,$C0,$CD,$16,$CD,$19,$00,$00,$00,$00,$00,$00,$00,$00,$00,
$00,$00,$00,$00,$00,$00,$00,$00,$00,$00,$00,$00,$00,$00,$00,$00,
$00,$00,$00,$00,$00,$00,$00,$00,$00,$00,$00,$00,$00,$00,$00,$00,
$00,$00,$00,$00,$00,$00,$00,$00,$00,$00,$00,$00,$00,$00,$00,$00,
$00,$00,$00,$00,$00,$00,$00,$00,$00,$00,$00,$00,$00,$00,$00,$00,
$0D,$0A,$0A,$4E,$6F,$6E,$2D,$53,$79,$73,$74,$65,$6D,$20,$64,$69,
$73,$6B,$20,$6F,$72,$20,$64,$69,$73,$6B,$20,$65,$72,$72,$6F,$72,
$0D,$0A,$52,$65,$70,$6C,$61,$63,$65,$20,$61,$6E,$64,$20,$73,$74,
$72,$69,$6B,$65,$20,$61,$6E,$79,$20,$6B,$65,$79,$20,$77,$68,$65,
$6E,$20,$72,$65,$61,$64,$79,$0D,$0A,$00,$55,$4E,$49,$44,$4F,$53,
$2E,$43,$4D,$44,$00,$00,$00,$00,$00,$00,$00,$00,$00,$00,$00,$00,
$00,$00,$00,$00,$00,$00,$00,$00,$00,$00,$00,$00,$00,$00,$00,$00,
$00,$00,$00,$00,$00,$00,$00,$00,$00,$00,$00,$00,$00,$00,$00,$00,
$00,$00,$00,$00,$00,$00,$00,$00,$00,$00,$00,$00,$00,$00,$00,$00,
$00,$00,$00,$00,$00,$00,$00,$00,$00,$00,$00,$00,$00,$00,$A5,$5A );

var
  drive: byte;
  retry: byte;
  err: word;
  sector: boot_arr;
  boot: boot_arr;

{$L call25}
function abs_read(var buff: boot_arr): word; external;
function abs_write(buff: boot_arr): word; external;



  procedure error(msg: string);
  begin
  writeln;
  write(msg); halt(1);
  end;

  procedure check_drv_type;
  type
    word_ptr = ^word;
  begin
  if word_ptr(@sector[11])^ <> 512 then
    error('Invalid sector size.');
  if word_ptr(@sector[$13])^ > $5104 then
    error('Drive size is more than 10M.');
  end;

  procedure init;
  var
    ch: char;
    s: string[80];
  begin
  boot:= boot_init;

  writeln;
  writeln('UniDOS MakeBoot Utility.      Version 3.00');
  writeln('Copyright (C) 1988-89  Software Research & Development Lab., Sofia');
  if paramcount < 1 then
    begin
    writeln(#10#13'Usage: MAKEBOOT <drive:>');
    halt(1);
    end
  else
    begin
    s:= paramstr(1); s[1]:= upcase(s[1]);
    if ((length(s) = 2) and (s[1] in ['A'.. 'Z'])) and (s[2] = ':') then
      drive:= ord(s[1]) - ord('A')
    else
      begin
      writeln('Invalid parameter.');
      halt(1);
      end;
    end;

  while keypressed do ch:= readkey;
  if drive > 1 then
    begin
    if drive = 2 then
      begin
      writeln;
      writeln('W A R N I N G  !'#7);
      writeln('This program will make drive C: unbootable on IBM PC');
      write('Do you want to continue ? '); ch:= upcase(readkey);
      if ch <> 'Y' then halt(0);
      writeln;
      write('Are you sure ? '); ch:= upcase(readkey);
      if ch <> 'Y' then halt(0);
      end;
    end
  else
    begin
    writeln;
    write('Insert disk in drive ');
    write(chr(drive + ord('A')), ':'#13#10'Strike any key when ready ');
    ch:= readkey;
    end;
  end;



begin
init;

retry:= max_retry;
repeat
  err:= abs_read(sector);
  dec(retry);
until (err = 0) or (retry = 0);
if err<>0 then
  begin
  writeln; write('Error reading disk.'); halt(1);
  end;

if drive > 1 then check_drv_type;

if sector[1] <> 0 then move(sector[11], boot[11], sector[1] - 9)
else move(sector[11], boot[11], 20);
retry:= max_retry;
repeat
  err:= abs_write(boot);
  dec(retry);
until (err = 0) or (retry = 0);
if err<>0 then
  begin
  writeln; write('Error writing disk.'); halt(1);
  end;
end.
