library ieee;
use ieee.std_logic_1164.all;
use ieee.std_logic_arith.all;
use ieee.std_logic_unsigned.all;
use ieee.numeric_std.all;

entity vgasync is
port(
	clk						: in std_logic;
	vga_hs              	: out std_logic;
	vga_vs              	: out std_logic;

	row						: out std_logic_vector(9 downto 0);
	column					: out std_logic_vector(9 downto 0);	
	enable					: out std_logic
);		
end vgasync;

architecture behavior of vgasync is
signal h_sync				: std_logic;
signal v_sync				: std_logic;
signal horizontal_en		: std_logic;
signal vertical_en		: std_logic;
signal h_cnt				: std_logic_vector(9 downto 0);
signal v_cnt				: std_logic_vector(9 downto 0);
begin
	enable <= horizontal_en and vertical_en;
	
	process
	begin
		wait until(clk'event) and (clk = '1');

		-- Generate Horizontal and Vertical Timing Signals for Video Signal

		--
		-- h_cnt counts pixels (640 + extra time for sync signals)
		--
		-- h_sync -------------------------------------------________-------- 
		-- h_cnt  0                                    640   659  755     799
		-- 
		if (h_cnt = 799) then
			h_cnt <= "0000000000";
		else
			h_cnt <= h_cnt + 1;
		end if;
		
		--
		-- Generate Horizontal Sync Signal using h_cnt 
		--
		if ((h_cnt <= 755) and (h_cnt >= 659)) then
			h_sync <= '0';
		else
			h_sync <= '1';
		end if;

		--
		-- v_cnt counts rows of pixels (480 + extra time for sync signals)
		-- v_sync ----------------------------------------_______------------ 
		-- v_cnt  0                               480     493-494         524
		--	

		if ((v_cnt >= 524) and (h_cnt >= 699)) then
			v_cnt <= "0000000000";
		elsif (h_cnt = 699) then
			v_cnt <= v_cnt + 1;
		end if;
	
		--
		-- Generate Vertical Sync Signal using v_cnt 
		--
		if ((v_cnt <= 494) and (v_cnt >= 493)) then
			v_sync <= '0';	
		else
			v_sync <= '1';
		end if;
		
		--
		-- Generate Video on Screen Signals for Pixel Data 
		--
		if (h_cnt <= 639) then
			horizontal_en <= '1';
		else
			horizontal_en <= '0';
		end if;

		if (v_cnt <= 479) then
			vertical_en <= '1';
		else
			vertical_en <= '0';
		end if;

		column <= h_cnt;
		row <= v_cnt;
		
		vga_hs <= h_sync;
		vga_vs <= v_sync;
	
	end process;
end behavior;
