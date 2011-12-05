library ieee;
use ieee.std_logic_1164.all;
use ieee.std_logic_arith.all;
use ieee.std_logic_unsigned.all;

entity pyldin2012 is
port(
	clk:	in std_logic;
	rst:	in std_logic;

--	vga_r               : out std_logic_vector(2 downto 0);
--	vga_g               : out std_logic_vector(2 downto 0);
--	vga_b               : out std_logic_vector(1 downto 0);
--	vga_hs              : out std_logic;
--	vga_vs              : out std_logic;
		
	sram_addr           : out   std_logic_vector(17 downto 0);
	sram_dq             : inout std_logic_vector(7 downto 0);
	sram_ce_n           : out   std_logic;
	sram_oe_n           : out   std_logic;
	sram_we_n           : out   std_logic;
	sram_ub_n           : out   std_logic;
	sram_lb_n           : out   std_logic;
	
	step:	in std_logic;
	ledseg: out std_logic_vector(7 downto 0);
	ledcom: out std_logic_vector(7 downto 0);
--	keys:	in std_logic_vector(2 downto 0)
);
end pyldin2012;

architecture pyldin_arch of pyldin2012 is
begin

end pyldin_arch;
