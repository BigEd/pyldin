--
-- PYLDIN 601 PS2 keyboard
--
library ieee;
use ieee.std_logic_1164.all;
use ieee.std_logic_arith.all;
use ieee.std_logic_unsigned.all;

entity keyboard is
port(
	rst							: in  std_logic;
	clk							: in  std_logic;
	ps2_clk						: in  std_logic;
	ps2_data						: in  std_logic;
	irq							: out std_logic;
	ack							: in  std_logic;
	cyr							: in  std_logic;
	data							: out std_logic_vector(7 downto 0)
);
end keyboard;

architecture keyboard_arch of keyboard is
type keys_array is array(0 to 143) of std_logic_vector(7 downto 0);

constant key_data				: keys_array := (
x"ff", x"d1", x"ff", x"cd", x"cb", x"c9", x"ca", x"d4", x"ff", x"d2", x"d0", x"ce", x"cc", x"ed", x"60", x"ff", 
x"ff", x"fb", x"ff", x"ff", x"ff", x"71", x"31", x"ff", x"ff", x"ff", x"7a", x"73", x"61", x"77", x"32", x"ff", 
x"ff", x"63", x"78", x"64", x"65", x"34", x"33", x"ff", x"ff", x"20", x"76", x"66", x"74", x"72", x"35", x"ff", 
x"ff", x"6e", x"62", x"68", x"67", x"79", x"36", x"ff", x"ff", x"ff", x"6d", x"6a", x"75", x"37", x"38", x"ff", 
x"ff", x"2c", x"6b", x"69", x"6f", x"30", x"39", x"ff", x"ff", x"2e", x"2f", x"6c", x"3b", x"70", x"2d", x"ff", 
x"ff", x"ff", x"27", x"ff", x"5b", x"3d", x"ff", x"ff", x"fc", x"ff", x"c0", x"5d", x"ff", x"5c", x"ff", x"ff", 
x"ff", x"ff", x"ff", x"ff", x"ff", x"ff", x"7f", x"ff", x"ff", x"c6", x"ff", x"c1", x"c5", x"ff", x"ff", x"ff", 
x"fa", x"ff", x"c3", x"ff", x"c2", x"c4", x"1b", x"ff", x"d3", x"ff", x"ff", x"ff", x"ff", x"ff", x"ff", x"ff", 
x"ff", x"ff", x"ff", x"cf", x"ff", x"ff", x"ff", x"ff", x"ff", x"ff", x"ff", x"ff", x"ff", x"ff", x"ff", x"ff" 
);

constant key_data_shift		: keys_array := (
x"ff", x"dd", x"ff", x"d9", x"d7", x"d5", x"d6", x"e0", x"ff", x"de", x"dc", x"da", x"d8", x"ee", x"7e", x"ff", 
x"ff", x"fb", x"ff", x"ff", x"ff", x"51", x"21", x"ff", x"ff", x"ff", x"5a", x"53", x"41", x"57", x"40", x"ff", 
x"ff", x"43", x"58", x"44", x"45", x"24", x"23", x"ff", x"ff", x"fd", x"56", x"46", x"54", x"52", x"25", x"ff", 
x"ff", x"4e", x"42", x"48", x"47", x"59", x"5e", x"ff", x"ff", x"ff", x"4d", x"4a", x"55", x"26", x"2a", x"ff", 
x"ff", x"3c", x"4b", x"49", x"4f", x"29", x"28", x"ff", x"ff", x"3e", x"3f", x"4c", x"3a", x"50", x"5f", x"ff", 
x"ff", x"ff", x"22", x"ff", x"7b", x"2b", x"ff", x"ff", x"fc", x"ff", x"c0", x"7d", x"ff", x"7c", x"ff", x"ff", 
x"ff", x"ff", x"ff", x"ff", x"ff", x"ff", x"7f", x"ff", x"ff", x"c6", x"ff", x"c5", x"c5", x"ff", x"ff", x"ff", 
x"fa", x"ff", x"c7", x"ff", x"c6", x"c8", x"1b", x"ff", x"df", x"ff", x"ff", x"ff", x"ff", x"ff", x"ff", x"ff", 
x"ff", x"ff", x"ff", x"db", x"ff", x"ff", x"ff", x"ff", x"ff", x"ff", x"ff", x"ff", x"ff", x"ff", x"ff", x"ff"
);

constant key_data_ctrl		: keys_array := (
x"ff", x"e9", x"ff", x"e5", x"e3", x"e1", x"e2", x"ec", x"ff", x"ea", x"e8", x"e6", x"e4", x"ed", x"60", x"ff", 
x"ff", x"fb", x"ff", x"ff", x"ff", x"11", x"31", x"ff", x"ff", x"ff", x"1a", x"13", x"01", x"17", x"00", x"ff", 
x"ff", x"03", x"18", x"04", x"05", x"34", x"33", x"ff", x"ff", x"fe", x"16", x"06", x"14", x"12", x"35", x"ff", 
x"ff", x"0e", x"02", x"08", x"07", x"19", x"1e", x"ff", x"ff", x"ff", x"0d", x"0a", x"15", x"37", x"38", x"ff", 
x"ff", x"2c", x"0b", x"09", x"0f", x"30", x"39", x"ff", x"ff", x"2e", x"2f", x"0c", x"3b", x"10", x"1f", x"ff", 
x"ff", x"ff", x"27", x"ff", x"1b", x"3d", x"ff", x"ff", x"fc", x"ff", x"f0", x"1d", x"ff", x"1c", x"ff", x"ff", 
x"ff", x"ff", x"ff", x"ff", x"ff", x"ff", x"ef", x"ff", x"ff", x"f6", x"ff", x"f1", x"f5", x"ff", x"ff", x"ff", 
x"fa", x"ff", x"f3", x"ff", x"f2", x"f4", x"1b", x"ff", x"eb", x"ff", x"ff", x"ff", x"ff", x"ff", x"ff", x"ff", 
x"ff", x"ff", x"ff", x"e7", x"ff", x"ff", x"ff", x"ff", x"ff", x"ff", x"ff", x"ff", x"ff", x"ff", x"ff", x"ff"
);

constant key_data_cshift	: keys_array := (
x"ff", x"e9", x"ff", x"e5", x"e3", x"e1", x"e2", x"ec", x"ff", x"ea", x"e8", x"e6", x"e4", x"ee", x"7e", x"ff", 
x"ff", x"fb", x"ff", x"ff", x"ff", x"11", x"31", x"ff", x"ff", x"ff", x"1a", x"13", x"01", x"17", x"00", x"ff", 
x"ff", x"03", x"18", x"04", x"05", x"34", x"33", x"ff", x"ff", x"fe", x"16", x"06", x"14", x"12", x"35", x"ff", 
x"ff", x"0e", x"02", x"08", x"07", x"19", x"1e", x"ff", x"ff", x"ff", x"0d", x"0a", x"15", x"37", x"38", x"ff", 
x"ff", x"3c", x"0b", x"09", x"0f", x"30", x"39", x"ff", x"ff", x"3e", x"3f", x"0c", x"3a", x"10", x"1f", x"ff", 
x"ff", x"ff", x"22", x"ff", x"1b", x"2b", x"ff", x"ff", x"fc", x"ff", x"f0", x"1d", x"ff", x"1c", x"ff", x"ff", 
x"ff", x"ff", x"ff", x"ff", x"ff", x"ff", x"ef", x"ff", x"ff", x"f6", x"ff", x"f5", x"f5", x"ff", x"ff", x"ff", 
x"fa", x"ff", x"f7", x"ff", x"f6", x"f8", x"1b", x"ff", x"eb", x"ff", x"ff", x"ff", x"ff", x"ff", x"ff", x"ff", 
x"ff", x"ff", x"ff", x"e7", x"ff", x"ff", x"ff", x"ff", x"ff", x"ff", x"ff", x"ff", x"ff", x"ff", x"ff", x"ff"
);

constant key_data_cyr		: keys_array := (
x"ff", x"d1", x"ff", x"cd", x"cb", x"c9", x"ca", x"d4", x"ff", x"d2", x"d0", x"ce", x"cc", x"ed", x"5b", x"ff", 
x"ff", x"fb", x"ff", x"ff", x"ff", x"a9", x"31", x"ff", x"ff", x"ff", x"bf", x"bb", x"b4", x"b6", x"32", x"ff", 
x"ff", x"b1", x"b7", x"a2", x"b3", x"34", x"33", x"ff", x"ff", x"20", x"ac", x"a0", x"a5", x"aa", x"35", x"ff", 
x"ff", x"b2", x"a8", x"b0", x"af", x"ad", x"36", x"ff", x"ff", x"ff", x"bc", x"ae", x"a3", x"37", x"38", x"ff", 
x"ff", x"a1", x"ab", x"b8", x"b9", x"30", x"39", x"ff", x"ff", x"be", x"2e", x"a4", x"a6", x"a7", x"2d", x"ff", 
x"ff", x"ff", x"bd", x"ff", x"b5", x"3d", x"ff", x"ff", x"fc", x"ff", x"c0", x"ba", x"ff", x"5c", x"ff", x"ff", 
x"ff", x"ff", x"ff", x"ff", x"ff", x"ff", x"7f", x"ff", x"ff", x"c6", x"ff", x"c1", x"c5", x"ff", x"ff", x"ff", 
x"fa", x"ff", x"c3", x"ff", x"c2", x"c4", x"1b", x"ff", x"d3", x"ff", x"ff", x"ff", x"ff", x"ff", x"ff", x"ff", 
x"ff", x"ff", x"ff", x"cf", x"ff", x"ff", x"ff", x"ff", x"ff", x"ff", x"ff", x"ff", x"ff", x"ff", x"ff", x"ff"
);

constant key_data_scyr		: keys_array := (
x"ff", x"dd", x"ff", x"d9", x"d7", x"d5", x"d6", x"e0", x"ff", x"de", x"dc", x"da", x"d8", x"ee", x"5d", x"ff", 
x"ff", x"fb", x"ff", x"ff", x"ff", x"89", x"21", x"ff", x"ff", x"ff", x"9f", x"9b", x"94", x"96", x"22", x"ff", 
x"ff", x"91", x"97", x"82", x"93", x"3b", x"23", x"ff", x"ff", x"fd", x"8c", x"80", x"85", x"8a", x"25", x"ff", 
x"ff", x"92", x"88", x"90", x"8f", x"8d", x"3a", x"ff", x"ff", x"ff", x"9c", x"8e", x"83", x"3f", x"2a", x"ff", 
x"ff", x"81", x"8b", x"98", x"99", x"29", x"28", x"ff", x"ff", x"9e", x"2c", x"84", x"86", x"87", x"5f", x"ff", 
x"ff", x"ff", x"9d", x"ff", x"95", x"2b", x"ff", x"ff", x"fc", x"ff", x"c0", x"9a", x"ff", x"2f", x"ff", x"ff", 
x"ff", x"ff", x"ff", x"ff", x"ff", x"ff", x"7f", x"ff", x"ff", x"c6", x"ff", x"c5", x"c5", x"ff", x"ff", x"ff", 
x"fa", x"ff", x"c7", x"ff", x"c6", x"c8", x"1b", x"ff", x"df", x"ff", x"ff", x"ff", x"ff", x"ff", x"ff", x"ff", 
x"ff", x"ff", x"ff", x"db", x"ff", x"ff", x"ff", x"ff", x"ff", x"ff", x"ff", x"ff", x"ff", x"ff", x"ff", x"ff"
);

signal clkdiv					: std_logic_vector(12 downto 0);
signal pclk						: std_logic;
signal dff1						: std_logic := '0';
signal dff2						: std_logic := '0';
signal k_clk					: std_logic := '0';
signal k_data					: std_logic := '0';
signal bcount					: std_logic_vector(3 downto 0) := "0000";
signal scancode				: std_logic_vector(7 downto 0);
signal code						: std_logic_vector(7 downto 0);
signal skipcode				: std_logic := '0';
signal ready					: std_logic := '0';

signal shiftkey				: std_logic := '0';
signal ctrlkey					: std_logic := '0';
begin
	clkdivider: process(clk)
	begin
		if (clk'event and clk = '1') then
			clkdiv <= clkdiv + 1;
		end if;
	end process;

	pclk <= clkdiv(2);
		
	process(rst, pclk, ps2_clk, ps2_data)
	begin
--		if (rst = '1') then
--			dff1 <= '0';
--			dff2 <= '0';
--			k_clk <= '0';
--			k_data <= '0';
--		else
			if (pclk'event and pclk = '1') then
				dff1 <= ps2_data;
				k_data <= dff1;
				dff2 <= ps2_clk;
				k_clk <= dff2;
			end if;
--		end if;
	end process;
	
	process (k_clk, k_data, ack, ready)
	begin
		if (ack = '1' and ready = '1') then
			ready <= '0';
		else
		if (k_clk'event and k_clk = '0') then
			case bcount is
				when "0000" =>
					bcount <= bcount + 1;
				when "1001" =>
					bcount <= bcount + 1;
				when "1010" =>
					if (skipcode = '1') then
						skipcode <= '0';
						if (scancode = "00010010") then -- shift released
							shiftkey <= '0';
						elsif (scancode = "00010100") then -- control released
							ctrlkey <= '0';
						end if;
					elsif (scancode = "11110000") then
						skipcode <= '1';
					elsif (scancode = "00010010") then -- shift pressed
						shiftkey <= '1';
					elsif (scancode = "00010100") then -- control pressed
						ctrlkey <= '1';
					elsif (scancode /= "11100000") then	-- ignore extended code
						ready <= '1';
						if ((shiftkey = '1') and (ctrlkey = '1')) then
							code <= key_data_cshift(conv_integer(scancode));
						elsif (shiftkey = '1') then
							if (cyr = '0') then
								code <= key_data_shift(conv_integer(scancode));
							else
								code <= key_data_scyr(conv_integer(scancode));
							end if;
						elsif (ctrlkey = '1') then
							code <= key_data_ctrl(conv_integer(scancode));
						else
							if (cyr = '0') then
								code <= key_data(conv_integer(scancode));
							else
								code <= key_data_cyr(conv_integer(scancode));
							end if;
						end if;
					end if;
					bcount <= "0000";
				when others =>
					scancode(6 downto 0) <= scancode(7 downto 1);
					scancode(7) <= k_data;
					bcount <= bcount + 1;
			end case;
		end if;
		end if;
	end process;

	data <= code;
	irq <= ready;
end keyboard_arch;
