library ieee;
use ieee.std_logic_1164.all;
use ieee.std_logic_arith.all;
use ieee.std_logic_unsigned.all;

entity vgasync is
port(
	clk						: in std_logic;
	vga_clk25				: out std_logic;
	vga_hs              	: out std_logic;
	vga_vs              	: out std_logic;

	row						: out std_logic_vector(9 downto 0);
	column					: out std_logic_vector(9 downto 0);	
	enable					: out std_logic
);		
end vgasync;

architecture behavior of vgasync is
signal clk25				: std_logic;
signal h_sync				: std_logic;
signal v_sync				: std_logic;
signal horizontal_en		: std_logic;
signal vertical_en		: std_logic;
signal h_cnt				: std_logic_vector(9 downto 0);
signal v_cnt				: std_logic_vector(9 downto 0);
begin
	enable <= horizontal_en and vertical_en;
	vga_clk25 <= clk25;
	
	--Generate 25Mhz Clock
	process (clk)
	begin
		if clk'event and clk='1' then
			if (clk25 = '0')then
				clk25 <= '1' after 2 ns;
			else
				clk25 <= '0' after 2 ns;
			end if;
		end if;
	end process;	

	process
	variable cnt: integer range 0 to 25000000;
	begin
		wait until(clk25'event) and (clk25 = '1');
		if (cnt = 25000000) then
			cnt := 0;
		else
			cnt := cnt  + 1;
		end if;

		-- reset horisontal counter
		if (h_cnt = 799) then
			h_cnt <= "0000000000";
		else
			h_cnt <= h_cnt + 1;
		end if;

		-- generate horisontal sync
		if ((h_cnt <= 755) and (h_cnt >= 659)) then
			h_sync <= '0';
		else
			h_sync <= '1';
		end if;

		-- reset vertical counter
		if ((v_cnt >= 524) and (h_cnt >= 699)) then
			v_cnt <= "0000000000";
		elsif (h_cnt = 699) then
			v_cnt <= v_cnt + 1;
		end if;
	
		-- generate vertical sync
		if ((v_cnt <= 494) and (v_cnt >= 493)) then
			v_sync <= '0';	
		else
			v_sync <= '1';
		end if;
	
		-- generate horizontal data
		if (h_cnt <= 639) then
			horizontal_en <= '1';
			column <= h_cnt;
		else
			horizontal_en <= '0';
		end if;
	
		-- generate vertical data
		if (v_cnt <= 479) then
			vertical_en <= '1';
			row <= v_cnt;
		else
			vertical_en <= '0';
		end if;
		
		vga_hs <= h_sync;
		vga_vs <= v_sync;
	
	end process;
end behavior;