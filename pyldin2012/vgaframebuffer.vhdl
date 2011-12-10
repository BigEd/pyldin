library ieee;
use ieee.std_logic_1164.all;
use ieee.std_logic_arith.all;
use ieee.std_logic_unsigned.all;

entity vgaframebuffer is
port(
	clk						: in std_logic;
	enable					: in std_logic;
	mode						: in std_logic;

	addr_in					: in std_logic_vector(16 downto 0);
	addr_base				: in std_logic_vector(15 downto 0);
	addr_out					: out std_logic_vector(15 downto 0);
	data_in					: in std_logic_vector( 7 downto 0);

	vga_r               	: out std_logic_vector(2 downto 0);
	vga_g               	: out std_logic_vector(2 downto 0);
	vga_b               	: out std_logic_vector(1 downto 0)
);
end vgaframebuffer;

architecture vgaframebuffer_arch of vgaframebuffer is
signal data					: std_logic_vector( 7 downto 0);
signal video_pixel		: std_logic;
begin
	process(clk, addr_base, addr_in)
	begin
		addr_out <= addr_base + addr_in(16 downto 3);

		if (clk'event and clk = '0') then
			if (addr_in(2 downto 0) = "000") then
				data <= data_in;
			else
				data(7 downto 1) <= data(6 downto 0);
			end if;
			video_pixel <= data(7);
			if (enable = '1') then
				vga_r(2) <= video_pixel; 
				vga_g(2) <= video_pixel;
				vga_b(1) <= video_pixel;
			else
				vga_r <= "000"; 
				vga_g <= "000";
				vga_b <= "00";
			end if;
		end if;
	end process;
end vgaframebuffer_arch;
