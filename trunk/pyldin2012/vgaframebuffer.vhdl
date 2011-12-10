library ieee;
use ieee.std_logic_1164.all;
use ieee.std_logic_arith.all;
use ieee.std_logic_unsigned.all;

entity vgaframebuffer is
port(
	clk						: in std_logic;
	enable					: in std_logic;
	mode						: in std_logic;

	row						: in std_logic_vector(9 downto 0);
	column					: in std_logic_vector(9 downto 0);	

	addr_base				: in std_logic_vector(15 downto 0);
	addr_out					: out std_logic_vector(15 downto 0);
	data_in					: in std_logic_vector( 7 downto 0);

	vga_r               	: out std_logic_vector(2 downto 0);
	vga_g               	: out std_logic_vector(2 downto 0);
	vga_b               	: out std_logic_vector(1 downto 0)
);
end vgaframebuffer;

architecture vgaframebuffer_arch of vgaframebuffer is
signal video_addr			: std_logic_vector(16 downto 0);
signal data					: std_logic_vector( 7 downto 0);
signal pixel				: std_logic;
begin
	process(clk, addr_base, video_addr, row, column)
	begin
--		if (mode = '1') then
			video_addr <= std_logic_vector("101000000"*row(8 downto 1) + column(9 downto 1));
			addr_out <= addr_base + video_addr(16 downto 3);
--		else
--			addr_out <= addr_base + addr_in();
--		end if;

		if (clk'event and clk = '0') then
			if (video_addr(2 downto 0) = "000") then
				data <= data_in;
			else
				data(7 downto 1) <= data(6 downto 0);
			end if;
			pixel <= data(7);
			if (enable = '1') then
				vga_r(2) <= pixel; 
				vga_g(2) <= pixel;
				vga_b(1) <= pixel;
			else
				vga_r <= "000"; 
				vga_g <= "000";
				vga_b <= "00";
			end if;
		end if;
	end process;
end vgaframebuffer_arch;
