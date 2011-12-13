library ieee;
use ieee.std_logic_1164.all;
use ieee.std_logic_arith.all;
use ieee.std_logic_unsigned.all;

entity SRAM is
port(
	clk						: in std_logic;
	rst						: in std_logic;

	sram_addr           	: out   std_logic_vector(17 downto 0);
	sram_dq             	: inout std_logic_vector(15 downto 0);
	sram_ce_n           	: out   std_logic;
	sram_oe_n           	: out   std_logic;
	sram_we_n           	: out   std_logic;
	sram_ub_n           	: out   std_logic;
	sram_lb_n           	: out   std_logic;

	cs							: in std_logic;
	rw							: in std_logic;
	addr        			: in std_logic_vector(15 downto 0);
	data_in     			: in std_logic_vector(7 downto 0);
	data_out   			 	: out std_logic_vector(7 downto 0)
);
end SRAM;

architecture SRAM_arch of SRAM is
signal ram_wr				: std_logic; -- memory write enable
begin
	process( clk, rst, addr, rw, data_in, cs, ram_wr, sram_dq )
	begin
		sram_ce_n <= '0'; -- not(cs and rst); -- put '0' to enable chip all time (no powersave mode)
		sram_oe_n <= not(rw and cs and rst);
		ram_wr    <= not(cs and (not rw) and clk);
		sram_we_n <= ram_wr;
		sram_lb_n <= not addr(0);
		sram_ub_n <= addr(0);
		sram_addr(17 downto 16) <= "00";
		sram_addr(15 downto 0 ) <= addr(15 downto 0);

		if (ram_wr = '0' and addr(0) = '0') then
			sram_dq(15 downto 8) <= data_in;
		else
			sram_dq(15 downto 8)  <= "ZZZZZZZZ";
		end if;

		if (ram_wr = '0' and addr(0) = '1') then
			sram_dq(7 downto 0) <= data_in;
		else
			sram_dq(7 downto 0)  <= "ZZZZZZZZ";
		end if;

		if (addr(0) = '0') then
			data_out <= sram_dq(15 downto 8);
		else
			data_out <= sram_dq(7 downto 0);
		end if;
	end process;
end SRAM_arch;
