library ieee;
use ieee.std_logic_1164.all;
use ieee.std_logic_arith.all;
use ieee.std_logic_unsigned.all;

entity vgachargen is
port(
	addr   	: in  std_logic_vector(10 downto 0);
	data		: out std_logic_vector(7 downto 0)
);
end vgachargen;

architecture basic of vgachargen is
constant width   : integer := 8;
constant memsize : integer := 2048;

type rom_array is array(0 to memsize-1) of std_logic_vector(width-1 downto 0);

constant rom_data : rom_array := (
x"00", x"00", x"00", x"00", x"00", x"00", x"00", x"00", x"08", x"14", x"22", x"22", x"3e", x"22", x"22", x"00", 
x"3e", x"41", x"55", x"41", x"5d", x"49", x"41", x"3e", x"3e", x"20", x"20", x"3c", x"22", x"22", x"3c", x"00", 
x"3e", x"7f", x"6b", x"7f", x"63", x"77", x"7f", x"3e", x"3c", x"22", x"22", x"3c", x"22", x"22", x"3c", x"00", 
x"36", x"7f", x"7f", x"7f", x"3e", x"1c", x"08", x"00", x"3e", x"20", x"20", x"20", x"20", x"20", x"20", x"00", 
x"08", x"1c", x"3e", x"7f", x"3e", x"1c", x"08", x"00", x"1e", x"12", x"12", x"12", x"12", x"12", x"3f", x"21", 
x"08", x"1c", x"1c", x"6b", x"7f", x"6b", x"08", x"1c", x"3e", x"20", x"20", x"38", x"20", x"20", x"3e", x"00", 
x"08", x"1c", x"3e", x"7f", x"7f", x"3e", x"08", x"1c", x"49", x"2a", x"1c", x"1c", x"2a", x"49", x"49", x"00", 
x"00", x"00", x"18", x"3c", x"3c", x"18", x"00", x"00", x"3c", x"02", x"02", x"1c", x"02", x"02", x"3c", x"00", 
x"ff", x"ff", x"e7", x"c3", x"c3", x"e7", x"ff", x"ff", x"22", x"22", x"26", x"2a", x"32", x"22", x"22", x"00", 
x"00", x"3c", x"66", x"42", x"42", x"66", x"3c", x"00", x"2a", x"22", x"26", x"2a", x"32", x"22", x"22", x"00", 
x"ff", x"c3", x"99", x"bd", x"bd", x"99", x"c3", x"ff", x"22", x"24", x"28", x"30", x"28", x"24", x"22", x"00", 
x"0f", x"03", x"05", x"39", x"48", x"48", x"30", x"00", x"0e", x"12", x"12", x"12", x"12", x"12", x"22", x"00", 
x"1c", x"22", x"22", x"1c", x"08", x"1c", x"08", x"00", x"22", x"36", x"2a", x"2a", x"22", x"22", x"22", x"00", 
x"0f", x"09", x"0f", x"08", x"08", x"38", x"30", x"00", x"22", x"22", x"22", x"3e", x"22", x"22", x"22", x"00", 
x"1f", x"11", x"1f", x"11", x"11", x"17", x"76", x"60", x"1c", x"22", x"22", x"22", x"22", x"22", x"1c", x"00", 
x"08", x"2a", x"1c", x"77", x"1c", x"2a", x"08", x"00", x"3e", x"22", x"22", x"22", x"22", x"22", x"22", x"00", 
x"10", x"18", x"1c", x"1e", x"1c", x"18", x"10", x"00", x"3c", x"22", x"22", x"3c", x"20", x"20", x"20", x"00", 
x"04", x"0c", x"1c", x"3c", x"1c", x"0c", x"04", x"00", x"1c", x"22", x"20", x"20", x"20", x"22", x"1c", x"00", 
x"08", x"1c", x"3e", x"08", x"3e", x"1c", x"08", x"00", x"3e", x"08", x"08", x"08", x"08", x"08", x"08", x"00", 
x"14", x"14", x"14", x"14", x"00", x"00", x"14", x"00", x"22", x"22", x"22", x"1e", x"02", x"02", x"3c", x"00", 
x"3f", x"49", x"49", x"29", x"09", x"09", x"09", x"00", x"08", x"3e", x"49", x"49", x"49", x"3e", x"08", x"00", 
x"1c", x"20", x"1c", x"22", x"22", x"1c", x"02", x"1c", x"22", x"22", x"14", x"08", x"14", x"22", x"22", x"00", 
x"00", x"00", x"00", x"00", x"ff", x"ff", x"ff", x"ff", x"24", x"24", x"24", x"24", x"24", x"24", x"3e", x"02", 
x"08", x"1c", x"3e", x"08", x"3e", x"1c", x"08", x"7f", x"22", x"22", x"22", x"1e", x"02", x"02", x"02", x"00", 
x"08", x"1c", x"3e", x"08", x"08", x"08", x"08", x"08", x"2a", x"2a", x"2a", x"2a", x"2a", x"2a", x"3e", x"00", 
x"08", x"08", x"08", x"08", x"08", x"3e", x"1c", x"08", x"2a", x"2a", x"2a", x"2a", x"2a", x"2a", x"3f", x"01", 
x"00", x"08", x"0c", x"fe", x"0c", x"08", x"00", x"00", x"30", x"10", x"10", x"1e", x"11", x"11", x"1e", x"00", 
x"00", x"10", x"30", x"7f", x"30", x"10", x"00", x"00", x"21", x"21", x"21", x"39", x"25", x"25", x"39", x"00", 
x"00", x"40", x"40", x"40", x"7f", x"00", x"00", x"00", x"20", x"20", x"20", x"3c", x"22", x"22", x"3c", x"00", 
x"00", x"14", x"36", x"7f", x"36", x"14", x"00", x"00", x"1c", x"22", x"02", x"1e", x"02", x"22", x"1c", x"00", 
x"08", x"1c", x"3e", x"7f", x"00", x"00", x"00", x"00", x"26", x"29", x"29", x"39", x"29", x"29", x"26", x"00", 
x"00", x"00", x"00", x"00", x"7f", x"3e", x"1c", x"08", x"1e", x"22", x"22", x"1e", x"0a", x"12", x"22", x"00", 
x"00", x"00", x"00", x"00", x"00", x"00", x"00", x"00", x"00", x"00", x"1c", x"02", x"1e", x"22", x"1f", x"00", 
x"08", x"08", x"08", x"08", x"08", x"00", x"08", x"00", x"1e", x"20", x"3c", x"22", x"22", x"22", x"1c", x"00", 
x"14", x"14", x"00", x"00", x"00", x"00", x"00", x"00", x"00", x"00", x"3c", x"22", x"3c", x"22", x"3c", x"00", 
x"14", x"14", x"3e", x"14", x"3e", x"14", x"14", x"00", x"00", x"00", x"3c", x"20", x"20", x"20", x"20", x"00", 
x"08", x"1e", x"28", x"1c", x"0a", x"3c", x"08", x"00", x"00", x"00", x"1e", x"0a", x"0a", x"12", x"3f", x"21", 
x"00", x"31", x"32", x"04", x"08", x"13", x"23", x"00", x"00", x"00", x"1c", x"22", x"3e", x"20", x"1e", x"00", 
x"18", x"24", x"24", x"18", x"25", x"22", x"1d", x"00", x"00", x"00", x"2a", x"1c", x"08", x"1c", x"2a", x"00", 
x"08", x"08", x"00", x"00", x"00", x"00", x"00", x"00", x"00", x"00", x"3c", x"02", x"0c", x"02", x"3c", x"00", 
x"04", x"08", x"10", x"10", x"10", x"08", x"04", x"00", x"00", x"00", x"22", x"26", x"2a", x"32", x"22", x"00", 
x"10", x"08", x"04", x"04", x"04", x"08", x"10", x"00", x"00", x"08", x"22", x"26", x"2a", x"32", x"22", x"00", 
x"00", x"08", x"2a", x"1c", x"1c", x"2a", x"08", x"00", x"00", x"00", x"26", x"28", x"38", x"24", x"22", x"00", 
x"00", x"08", x"08", x"3e", x"08", x"08", x"00", x"00", x"00", x"00", x"0e", x"12", x"12", x"12", x"32", x"00", 
x"00", x"00", x"00", x"00", x"00", x"08", x"08", x"10", x"00", x"00", x"22", x"36", x"2a", x"22", x"22", x"00", 
x"00", x"00", x"00", x"3e", x"00", x"00", x"00", x"00", x"00", x"00", x"22", x"22", x"3e", x"22", x"22", x"00", 
x"00", x"00", x"00", x"00", x"00", x"00", x"08", x"00", x"00", x"00", x"1c", x"22", x"22", x"22", x"1c", x"00", 
x"00", x"01", x"02", x"04", x"08", x"10", x"20", x"00", x"00", x"00", x"3e", x"22", x"22", x"22", x"22", x"00", 
x"1c", x"22", x"26", x"2a", x"32", x"22", x"1c", x"00", x"00", x"00", x"3c", x"22", x"22", x"3c", x"20", x"20", 
x"08", x"18", x"08", x"08", x"08", x"08", x"1c", x"00", x"00", x"00", x"1e", x"20", x"20", x"20", x"1e", x"00", 
x"1c", x"22", x"02", x"0c", x"10", x"20", x"3e", x"00", x"00", x"00", x"3e", x"08", x"08", x"08", x"08", x"00", 
x"3e", x"02", x"04", x"0c", x"02", x"22", x"1c", x"00", x"00", x"00", x"22", x"22", x"22", x"1e", x"02", x"3c", 
x"04", x"0c", x"14", x"24", x"3e", x"04", x"04", x"00", x"00", x"08", x"1c", x"2a", x"2a", x"2a", x"1c", x"08", 
x"3e", x"20", x"3c", x"02", x"02", x"22", x"1c", x"00", x"00", x"00", x"22", x"14", x"08", x"14", x"22", x"00", 
x"0e", x"10", x"20", x"3c", x"22", x"22", x"1c", x"00", x"00", x"00", x"24", x"24", x"24", x"24", x"3e", x"02", 
x"3e", x"02", x"04", x"08", x"10", x"10", x"10", x"00", x"00", x"00", x"22", x"22", x"1e", x"02", x"02", x"00", 
x"1c", x"22", x"22", x"1c", x"22", x"22", x"1c", x"00", x"00", x"00", x"2a", x"2a", x"2a", x"2a", x"3e", x"00", 
x"1c", x"22", x"22", x"1e", x"02", x"04", x"38", x"00", x"00", x"00", x"2a", x"2a", x"2a", x"2a", x"3f", x"01", 
x"00", x"00", x"08", x"00", x"08", x"00", x"00", x"00", x"00", x"00", x"30", x"10", x"1e", x"11", x"1e", x"00", 
x"00", x"00", x"08", x"00", x"08", x"08", x"10", x"00", x"00", x"00", x"21", x"21", x"39", x"25", x"39", x"00", 
x"02", x"04", x"08", x"10", x"08", x"04", x"02", x"00", x"00", x"00", x"20", x"20", x"3c", x"22", x"3c", x"00", 
x"00", x"00", x"3e", x"00", x"3e", x"00", x"00", x"00", x"00", x"00", x"1c", x"22", x"0e", x"22", x"1c", x"00", 
x"10", x"08", x"04", x"02", x"04", x"08", x"10", x"00", x"00", x"00", x"26", x"29", x"39", x"29", x"26", x"00", 
x"1c", x"22", x"02", x"04", x"08", x"00", x"08", x"00", x"00", x"00", x"1e", x"22", x"1e", x"0a", x"32", x"00", 
x"1c", x"22", x"2a", x"2e", x"2c", x"20", x"1e", x"00", x"08", x"08", x"08", x"0f", x"00", x"00", x"00", x"00", 
x"08", x"14", x"22", x"22", x"3e", x"22", x"22", x"00", x"08", x"08", x"08", x"ff", x"00", x"00", x"00", x"00", 
x"3c", x"22", x"22", x"3c", x"22", x"22", x"3c", x"00", x"00", x"00", x"00", x"ff", x"08", x"08", x"08", x"08", 
x"1c", x"22", x"20", x"20", x"20", x"22", x"1c", x"00", x"08", x"08", x"08", x"0f", x"08", x"08", x"08", x"08", 
x"3c", x"22", x"22", x"22", x"22", x"22", x"3c", x"00", x"00", x"00", x"00", x"ff", x"00", x"00", x"00", x"00", 
x"3e", x"20", x"20", x"3c", x"20", x"20", x"3e", x"00", x"08", x"08", x"08", x"ff", x"08", x"08", x"08", x"08", 
x"3e", x"20", x"20", x"3c", x"20", x"20", x"20", x"00", x"14", x"14", x"f4", x"04", x"f4", x"14", x"14", x"14", 
x"1e", x"20", x"20", x"26", x"22", x"22", x"1e", x"00", x"14", x"14", x"14", x"14", x"14", x"14", x"14", x"14", 
x"22", x"22", x"22", x"3e", x"22", x"22", x"22", x"00", x"14", x"14", x"17", x"10", x"1f", x"00", x"00", x"00", 
x"1c", x"08", x"08", x"08", x"08", x"08", x"1c", x"00", x"00", x"00", x"1f", x"10", x"17", x"14", x"14", x"14", 
x"02", x"02", x"02", x"02", x"02", x"22", x"1c", x"00", x"14", x"14", x"f7", x"00", x"ff", x"00", x"00", x"00", 
x"22", x"24", x"28", x"30", x"28", x"24", x"22", x"00", x"00", x"00", x"ff", x"00", x"f7", x"14", x"14", x"14", 
x"20", x"20", x"20", x"20", x"20", x"20", x"3e", x"00", x"14", x"14", x"17", x"10", x"17", x"14", x"14", x"14", 
x"22", x"36", x"2a", x"2a", x"22", x"22", x"22", x"00", x"00", x"00", x"ff", x"00", x"ff", x"00", x"00", x"00", 
x"22", x"22", x"32", x"2a", x"26", x"22", x"22", x"00", x"14", x"14", x"f7", x"00", x"f7", x"14", x"14", x"14", 
x"1c", x"22", x"22", x"22", x"22", x"22", x"1c", x"00", x"00", x"00", x"00", x"f8", x"08", x"08", x"08", x"08", 
x"3c", x"22", x"22", x"3c", x"20", x"20", x"20", x"00", x"11", x"44", x"11", x"44", x"11", x"44", x"11", x"44", 
x"1c", x"22", x"22", x"22", x"2a", x"24", x"1a", x"00", x"55", x"aa", x"55", x"aa", x"55", x"aa", x"55", x"aa", 
x"3c", x"22", x"22", x"3c", x"28", x"24", x"22", x"00", x"bb", x"ee", x"bb", x"ee", x"bb", x"ee", x"bb", x"ee", 
x"1c", x"22", x"20", x"1c", x"02", x"22", x"1c", x"00", x"08", x"08", x"08", x"08", x"08", x"08", x"08", x"08", 
x"3e", x"08", x"08", x"08", x"08", x"08", x"08", x"00", x"08", x"08", x"08", x"f8", x"08", x"08", x"08", x"08", 
x"22", x"22", x"22", x"22", x"22", x"22", x"1c", x"00", x"8a", x"8d", x"ca", x"a8", x"9b", x"88", x"88", x"00", 
x"22", x"22", x"22", x"22", x"22", x"14", x"08", x"00", x"3c", x"42", x"99", x"a1", x"a1", x"99", x"42", x"3c", 
x"22", x"22", x"22", x"2a", x"2a", x"36", x"22", x"00", x"00", x"00", x"fc", x"04", x"f4", x"14", x"14", x"14", 
x"22", x"22", x"14", x"08", x"14", x"22", x"22", x"00", x"14", x"14", x"f4", x"04", x"fc", x"00", x"00", x"00", 
x"22", x"22", x"14", x"08", x"08", x"08", x"08", x"00", x"08", x"08", x"08", x"f8", x"00", x"00", x"00", x"00", 
x"3e", x"02", x"04", x"08", x"10", x"20", x"3e", x"00", x"00", x"00", x"00", x"0f", x"08", x"08", x"08", x"08", 
x"3e", x"30", x"30", x"30", x"30", x"30", x"3e", x"00", x"ff", x"ff", x"ff", x"ff", x"ff", x"ff", x"ff", x"ff", 
x"00", x"20", x"10", x"08", x"04", x"02", x"00", x"00", x"00", x"00", x"00", x"00", x"ff", x"ff", x"ff", x"ff", 
x"3e", x"06", x"06", x"06", x"06", x"06", x"3e", x"00", x"f0", x"f0", x"f0", x"f0", x"f0", x"f0", x"f0", x"f0", 
x"00", x"00", x"08", x"14", x"22", x"00", x"00", x"00", x"0f", x"0f", x"0f", x"0f", x"0f", x"0f", x"0f", x"0f", 
x"00", x"00", x"00", x"00", x"00", x"00", x"00", x"ff", x"ff", x"ff", x"ff", x"ff", x"00", x"00", x"00", x"00", 
x"10", x"08", x"04", x"00", x"00", x"00", x"00", x"00", x"00", x"02", x"04", x"34", x"4c", x"48", x"36", x"00", 
x"00", x"00", x"1c", x"02", x"1e", x"22", x"1e", x"00", x"18", x"24", x"24", x"28", x"24", x"24", x"28", x"20", 
x"20", x"20", x"3c", x"22", x"22", x"22", x"3c", x"00", x"3e", x"22", x"20", x"20", x"20", x"20", x"20", x"00", 
x"00", x"00", x"1e", x"20", x"20", x"20", x"1e", x"00", x"00", x"00", x"3f", x"52", x"12", x"12", x"11", x"00", 
x"02", x"02", x"1e", x"22", x"22", x"22", x"1e", x"00", x"7e", x"22", x"10", x"08", x"10", x"22", x"7e", x"00", 
x"00", x"00", x"1c", x"22", x"3e", x"20", x"1e", x"00", x"00", x"00", x"1f", x"24", x"24", x"24", x"18", x"00", 
x"0c", x"12", x"10", x"38", x"10", x"10", x"10", x"00", x"00", x"22", x"22", x"36", x"2a", x"21", x"20", x"40", 
x"00", x"00", x"1c", x"22", x"22", x"1e", x"02", x"1c", x"20", x"16", x"09", x"08", x"18", x"18", x"18", x"18", 
x"20", x"20", x"3c", x"22", x"22", x"22", x"22", x"00", x"7f", x"08", x"1c", x"22", x"22", x"1c", x"08", x"7f", 
x"08", x"00", x"18", x"08", x"08", x"08", x"1c", x"00", x"0c", x"12", x"21", x"2d", x"21", x"12", x"0c", x"00", 
x"02", x"00", x"06", x"02", x"02", x"02", x"12", x"0c", x"00", x"1c", x"22", x"22", x"22", x"14", x"36", x"00", 
x"20", x"20", x"22", x"24", x"38", x"24", x"22", x"00", x"10", x"28", x"24", x"10", x"28", x"24", x"24", x"18", 
x"18", x"08", x"08", x"08", x"08", x"08", x"1c", x"00", x"00", x"00", x"00", x"36", x"49", x"49", x"36", x"00", 
x"00", x"00", x"36", x"2a", x"2a", x"2a", x"2a", x"00", x"1d", x"22", x"45", x"49", x"51", x"22", x"5c", x"00", 
x"00", x"00", x"3c", x"22", x"22", x"22", x"22", x"00", x"0e", x"10", x"20", x"3e", x"20", x"10", x"0e", x"00", 
x"00", x"00", x"1c", x"22", x"22", x"22", x"1c", x"00", x"1c", x"22", x"22", x"22", x"22", x"22", x"22", x"00", 
x"00", x"00", x"3c", x"22", x"22", x"3c", x"20", x"20", x"00", x"7e", x"00", x"7e", x"00", x"7e", x"00", x"00", 
x"00", x"00", x"1e", x"22", x"22", x"1e", x"02", x"02", x"08", x"08", x"3e", x"08", x"08", x"00", x"3e", x"00", 
x"00", x"00", x"2e", x"30", x"20", x"20", x"20", x"00", x"60", x"18", x"06", x"18", x"60", x"00", x"7e", x"00", 
x"00", x"00", x"1e", x"20", x"1c", x"02", x"3c", x"00", x"06", x"18", x"60", x"18", x"06", x"00", x"7e", x"00", 
x"10", x"10", x"38", x"10", x"10", x"12", x"0c", x"00", x"06", x"09", x"08", x"08", x"08", x"08", x"08", x"08", 
x"00", x"00", x"22", x"22", x"22", x"26", x"1a", x"00", x"08", x"08", x"08", x"08", x"08", x"08", x"48", x"30", 
x"00", x"00", x"22", x"22", x"22", x"14", x"08", x"00", x"18", x"18", x"00", x"7e", x"00", x"18", x"18", x"00", 
x"00", x"00", x"22", x"22", x"2a", x"2a", x"36", x"00", x"00", x"00", x"32", x"4c", x"00", x"32", x"4c", x"00", 
x"00", x"00", x"22", x"14", x"08", x"14", x"22", x"00", x"18", x"24", x"18", x"00", x"00", x"00", x"00", x"00", 
x"00", x"00", x"22", x"22", x"22", x"1e", x"02", x"1c", x"00", x"00", x"00", x"18", x"18", x"00", x"00", x"00", 
x"00", x"00", x"3e", x"04", x"08", x"10", x"3e", x"00", x"00", x"00", x"00", x"18", x"00", x"00", x"00", x"00", 
x"0e", x"18", x"18", x"30", x"18", x"18", x"0e", x"00", x"0f", x"08", x"08", x"08", x"48", x"28", x"18", x"08", 
x"08", x"08", x"08", x"00", x"08", x"08", x"08", x"00", x"70", x"48", x"48", x"48", x"48", x"00", x"00", x"00", 
x"38", x"0c", x"0c", x"06", x"0c", x"0c", x"38", x"00", x"30", x"48", x"10", x"20", x"78", x"00", x"00", x"00", 
x"1a", x"2c", x"00", x"00", x"00", x"00", x"00", x"00", x"00", x"00", x"3c", x"3c", x"3c", x"3c", x"00", x"00", 
x"08", x"08", x"14", x"14", x"22", x"22", x"41", x"7f", x"ff", x"81", x"81", x"81", x"81", x"81", x"81", x"ff"
);
begin
	data <= rom_data(conv_integer(addr));
end basic;