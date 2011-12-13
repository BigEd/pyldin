library ieee;
use ieee.std_logic_1164.all;
use ieee.std_logic_arith.all;
use ieee.std_logic_unsigned.all;

entity vgaframebuffer is
generic(
	READ_DELAY				: natural :=36
	);
port(
	rst						: in std_logic;
	clk						: in std_logic;
	pixel_clk				: in std_logic;
	enable					: out std_logic;
	mode						: in std_logic;

	addr_base				: in std_logic_vector(15 downto 0);
	addr_out					: out std_logic_vector(15 downto 0);
	data_in					: in std_logic_vector( 7 downto 0);

	vga_r               	: out std_logic_vector(2 downto 0);
	vga_g               	: out std_logic_vector(2 downto 0);
	vga_b               	: out std_logic_vector(1 downto 0);

	vga_hs              	: out std_logic;
	vga_vs              	: out std_logic
);
end vgaframebuffer;

architecture vgaframebuffer_arch of vgaframebuffer is
signal video_addr			: std_logic_vector(16 downto 0);
signal data					: std_logic_vector( 7 downto 0);
signal pixel				: std_logic;
signal char_addr			: std_logic_vector(10 downto 0);
signal char_data			: std_logic_vector( 7 downto 0);

signal h_sync				: std_logic;
signal v_sync				: std_logic;
signal horizontal_en		: std_logic;
signal vertical_en		: std_logic;
signal video_enable		: std_logic;
signal h_cnt				: std_logic_vector(9 downto 0);
signal v_cnt				: std_logic_vector(9 downto 0);
begin
	chargen: entity work.vgachargen port map (addr => char_addr, data => char_data);

	video_enable <= horizontal_en and vertical_en;
	enable <= video_enable;
	vga_hs <= h_sync;
	vga_vs <= v_sync;

	hsync: process (clk, rst)
	begin
		if (rst = '1') then
			h_cnt <= "0000000000";
		elsif (clk'event and clk = '1') then
			if (h_cnt = 799) then
				h_cnt <= "0000000000";
			else
				h_cnt <= h_cnt + 1;
			end if;

			if ((h_cnt <= (755 + READ_DELAY)) and (h_cnt >= (659 + READ_DELAY))) then
--			if ((h_cnt <= 787) and (h_cnt >= 691)) then
				h_sync <= '0';
			else
				h_sync <= '1';
			end if;
			
			if ((h_cnt >= (0 + READ_DELAY)) and (h_cnt <= (639 + READ_DELAY))) then
				horizontal_en <= '1';
			else
				horizontal_en <= '0';
			end if;
		end if;
	end process;
	
	vsync: process (h_sync, rst)
	begin
		if (rst = '1') then
			v_cnt <= "0000000000";
		elsif (h_sync'event and h_sync = '1') then
			if (v_cnt >= 524) then
				v_cnt <= "0000000000";
			else
				v_cnt <= v_cnt + 1;
			end if;

			if ((v_cnt <= 494) and (v_cnt >= 493)) then
				v_sync <= '0';	
			else
				v_sync <= '1';
			end if;
			
			if (v_cnt <= 479) then
				vertical_en <= '1';
			else
				vertical_en <= '0';
			end if;
		end if;
	end process;

	vaddrout: process(clk)
	begin
		if (clk'event and clk = '1') then
			if (mode = '1') then
				video_addr <= std_logic_vector("101000000"*v_cnt(8 downto 1) + h_cnt(9 downto 1));
				addr_out <= addr_base + video_addr(16 downto 3);
			else
				video_addr <= std_logic_vector("00000101000"*v_cnt(9 downto 4) + h_cnt(9 downto 4));
				addr_out <= addr_base + video_addr(15 downto 0);
			end if;
		end if;
	end process;
	
	vdataout: process(clk, pixel_clk, addr_base, video_addr, v_cnt, h_cnt, mode)
	begin
		if (pixel_clk'event and pixel_clk = '0') then
			if (h_cnt(3 downto 1) = "000") then
				if (mode = '1') then
					data <= data_in;
				else
					char_addr( 2 downto 0) <= v_cnt(3 downto 1);
					char_addr( 3) <= data_in(7);
					char_addr(10 downto 4) <= data_in(6 downto 0);
					data <= char_data;
				end if;
			else
				data(7 downto 1) <= data(6 downto 0);
			end if;
			pixel <= data(7);
			if (video_enable = '1') then
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
