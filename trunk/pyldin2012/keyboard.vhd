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
					if (scancode = "11110000") then
						skipcode <= '1';
					elsif (skipcode = '1') then
						skipcode <= '0';
--						code <= "00000000";
--					elsif (scancode /= "11100000") then
					else
						ready <= '1';
						code <= key_data(conv_integer(scancode));
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
