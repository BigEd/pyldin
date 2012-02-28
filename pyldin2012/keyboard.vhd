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
signal clkdiv					: std_logic_vector(12 downto 0);
signal pclk						: std_logic;
signal dff1						: std_logic;
signal dff2						: std_logic;
signal k_clk					: std_logic;
signal k_data					: std_logic;
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
		if (rst = '1') then
			dff1 <= '0';
			dff2 <= '0';
			k_clk <= '0';
			k_data <= '0';
		else
			if (pclk'event and pclk = '1') then
				dff1 <= ps2_data;
				k_data <= dff1;
				dff2 <= ps2_clk;
				k_clk <= dff2;
			end if;
		end if;
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
--						data <= "00000000";
--					elsif (scancode /= "11100000") then
					else
						ready <= '1';
						code <= scancode;
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
