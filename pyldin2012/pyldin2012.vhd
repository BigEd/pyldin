library ieee;
use ieee.std_logic_1164.all;
use ieee.std_logic_arith.all;
use ieee.std_logic_unsigned.all;

entity pyldin2012 is
port(
	clk						: in std_logic;
	rst						: in std_logic;

	vga_r               	: out std_logic_vector(2 downto 0);
	vga_g               	: out std_logic_vector(2 downto 0);
	vga_b               	: out std_logic_vector(1 downto 0);
	vga_hs              	: out std_logic;
	vga_vs              	: out std_logic;
		
	sram_addr           	: out   std_logic_vector(17 downto 0);
	sram_dq             	: inout std_logic_vector(15 downto 0);
	sram_ce_n           	: out   std_logic;
	sram_oe_n           	: out   std_logic;
	sram_we_n           	: out   std_logic;
	sram_ub_n           	: out   std_logic;
	sram_lb_n           	: out   std_logic;
	
	swt						: in std_logic;
	step						: in std_logic;
	ledseg					: out std_logic_vector(7 downto 0);
	ledcom					: out std_logic_vector(7 downto 0)
--	keys						: in std_logic_vector(2 downto 0)
);
end pyldin2012;

architecture pyldin_arch of pyldin2012 is
signal clk_cnt				: std_logic_vector(3 downto 0);
signal clk25				: std_logic;
signal clk125				: std_logic;
signal clk625				: std_logic;
signal rst_cnt				: std_logic_vector(3 downto 0) := "1111";
signal sys_rst				: std_logic := '1';
signal sys_clk				: std_logic;
signal vram_access		: std_logic;
signal pixel_clk			: std_logic;

-- cpu interface signals
signal cpu_reset       	: std_logic;
signal cpu_rw          	: std_logic;
signal cpu_vma         	: std_logic;
signal cpu_halt        	: std_logic;
signal cpu_hold        	: std_logic;
signal cpu_irq         	: std_logic;
signal cpu_nmi         	: std_logic;
signal cpu_addr        	: std_logic_vector(15 downto 0);
signal cpu_data_in     	: std_logic_vector(7 downto 0);
signal cpu_data_out    	: std_logic_vector(7 downto 0);

-- cpu test pins
signal test_alu			: std_logic_vector(15 downto 0); -- ALU output for timing constraints
signal test_cc				: std_logic_vector(7 downto 0);  -- Condition Code Outputs for timing constraints

-- rombios
signal rombios_cs       : std_logic;
signal rombios_data_out : std_logic_vector(7 downto 0);

-- ram
signal ram_cs          	: std_logic; -- memory chip select
signal ram_hold			: std_logic;
signal ram_data_out    	: std_logic_vector(7 downto 0);

-- mc6845
signal mc6845_addr		: std_logic_vector(7 downto 0);
signal mc6845_data_out	: std_logic_vector(7 downto 0);

-- video ram
signal vram_cs				: std_logic;
signal vram_rw				: std_logic;
signal vram_base_addr	: std_logic_vector(15 downto 0);
signal vram_addr			: std_logic_vector(15 downto 0);
signal vram_data_out		: std_logic_vector( 7 downto 0);
signal vram_data_in		: std_logic_vector( 7 downto 0);

-- ram mux
signal mux_ram_cs			: std_logic;
signal mux_ram_rw			: std_logic;
signal mux_ram_addr		: std_logic_vector(15 downto 0);
signal mux_ram_data_in	: std_logic_vector( 7 downto 0);
signal mux_ram_data_out	: std_logic_vector( 7 downto 0);
type type_states	is (Idle, Addr, Read, Write, WrtEnd);
signal ram_state			: type_states;

-- IO selector
signal ds0_cs				: std_logic;
signal ds1_cs				: std_logic;
signal ds2_cs				: std_logic;
signal ds3_cs				: std_logic;
signal ds4_cs				: std_logic;
signal ds5_cs				: std_logic;
signal ds6_cs				: std_logic;
signal ds7_cs				: std_logic;

-- video
signal video_row			: std_logic_vector(9 downto 0);
signal video_column		: std_logic_vector(10 downto 0);
signal video_en			: std_logic;
signal video_addr			: std_logic_vector(16 downto 0);
signal video_mode			: std_logic;

-- hex display
signal led_data			: std_logic_vector(31 downto 0);

-- DS5
signal ds5_data_in		: std_logic_vector(31 downto 0);
signal ds5_data_out		: std_logic_vector(7 downto 0);

-- hardware debugger
signal step_clk			: std_logic;
signal step_display		: std_logic_vector(31 downto 0);
signal step_debouncer	: std_logic_vector(24 downto 0);
begin

	cpu_hold <= ram_hold;

	clock25: process (clk, rst)
	begin
		if (clk'event and clk='1') then
			if (rst = '0') then
				rst_cnt <= "0100";
				sys_rst <= '1';
			else
				clk25 <= clk_cnt(0);
				clk125 <= clk_cnt(1);
				clk625 <= clk_cnt(2);
				clk_cnt <= clk_cnt + 1;
				if (clk625 = '1') then
					if (rst_cnt = "0000") then
						sys_rst <= '0';
					else
						rst_cnt <= rst_cnt - 1;
					end if;
				end if;
			end if;
		end if;
	end process;
	
	videocycle: process(clk)
	begin
		if (clk'event and clk = '0') then
			if (clk_cnt(1 downto 0) = "11") then
				pixel_clk <= '1';
			else
				pixel_clk <= '0';
			end if;
			if (clk_cnt(1 downto 0) = "11") or (clk_cnt(1 downto 0) = "00") then
				vram_access <= '1';
			else
				vram_access <= '0';
			end if;
		end if;
	end process;
	
	debugmode: process(swt, ds5_data_in, step_display, step_clk, clk25, clk125, clk625, vram_access)
	begin
		led_data <= ds5_data_in;
		sys_clk  <= clk25;
	end process;

	interrupts : process(rst, pixel_clk, vram_access, sys_rst, cpu_vma)
	begin
		cpu_halt  <= '0';
--		cpu_hold  <= '0'; -- cpu_vma and pixel_clk; --'0';
		cpu_irq   <= '0'; -- uart_irq or timer_irq;
		cpu_nmi   <= '0'; -- trap_irq;
		cpu_reset <= sys_rst; -- CPU reset is active high
	end process;
	
	mc6800 : entity work.cpu68 port map(
		clk		=> sys_clk,
		rst		=> cpu_reset,
		rw			=> cpu_rw,
		vma		=> cpu_vma,
		address	=> cpu_addr(15 downto 0),
		data_in	=> cpu_data_in,
		data_out	=> cpu_data_out,
		hold		=> cpu_hold,
		halt		=> cpu_halt,
		irq		=> cpu_irq,
		nmi		=> cpu_nmi,
		test_alu	=> test_alu,
		test_cc	=> test_cc
	);

	rombios : entity work.rombios_rom port map (
		addr		=> cpu_addr(11 downto 0),
		data		=> rombios_data_out
	);

	ram: entity work.SRAM port map (
		clk 			=> clk25,
		rst 			=> not cpu_reset,
		sram_addr 	=> sram_addr,
		sram_dq 		=> sram_dq,
		sram_ce_n 	=> sram_ce_n,
		sram_oe_n 	=> sram_oe_n,
		sram_we_n 	=> sram_we_n,
		sram_ub_n 	=> sram_ub_n,
		sram_lb_n 	=> sram_lb_n,
		cs 			=> mux_ram_cs,
		rw 			=> mux_ram_rw,
		addr 			=> mux_ram_addr,
		data_in 		=> mux_ram_data_in,
		data_out 	=> mux_ram_data_out
	);
	
	segdisplay : entity work.segleds port map(
		clk		=> clk,
		rst		=> not cpu_reset,
		ledseg	=> ledseg,
		ledcom	=> ledcom,
		data		=> led_data
	);
	
	ds5hexout : process (cpu_addr, cpu_rw, cpu_data_out, ds5_cs, ds5_data_in, sys_clk)
	begin
		if (sys_clk'event and sys_clk = '1') then
			if (ds5_cs = '1') then
				if (cpu_rw = '0') then
					case (cpu_addr(1 downto 0)) is
						when "00" => ds5_data_in(31 downto 24) <= cpu_data_out;
						when "01" => ds5_data_in(23 downto 16) <= cpu_data_out;
						when "10" => ds5_data_in(15 downto 8 ) <= cpu_data_out;
						when "11" => ds5_data_in( 7 downto 0 ) <= cpu_data_out;
					end case;
				else
					case (cpu_addr(1 downto 0)) is
						when "00" => ds5_data_out <= ds5_data_in(31 downto 24);
						when "01" => ds5_data_out <= ds5_data_in(23 downto 16);
						when "10" => ds5_data_out <= ds5_data_in(15 downto 8 );
						when "11" => ds5_data_out <= ds5_data_in( 7 downto 0 );
					end case;
				end if;
			end if;
		end if;
	end process;
	
	decode: process( cpu_addr, cpu_rw, cpu_vma, cpu_data_in,
					  rombios_cs, rombios_data_out,
				     ram_cs, ram_data_out,
					  mc6845_data_out,
					  ds5_data_out
					  )
	begin
      case cpu_addr(15 downto 12) is
			when "1111" =>											-- $Fxxx
				cpu_data_in <= rombios_data_out;				-- read ROM
				if (cpu_rw = '1') then
					rombios_cs  <= cpu_vma;
					ram_cs      <= '0';
				else
					rombios_cs  <= '0';
					ram_cs      <= cpu_vma;
				end if;
				ds0_cs <= '0'; ds1_cs <= '0'; ds2_cs <= '0'; ds3_cs <= '0'; ds4_cs <= '0'; ds5_cs <= '0'; ds6_cs <= '0'; ds7_cs <= '0';
			when "1110" =>											-- $Exxx
				if (cpu_addr(11 downto 8) = "0110") then	-- IO $E6Xxx selector
					case (cpu_addr(7 downto 5)) is
					when "000" =>
						ds0_cs <= cpu_vma; ds1_cs <= '0'; ds2_cs <= '0'; ds3_cs <= '0'; ds4_cs <= '0'; ds5_cs <= '0'; ds6_cs <= '0'; ds7_cs <= '0';
						cpu_data_in <= mc6845_data_out;
					when "001" =>
						ds0_cs <= '0'; ds1_cs <= cpu_vma; ds2_cs <= '0'; ds3_cs <= '0'; ds4_cs <= '0'; ds5_cs <= '0'; ds6_cs <= '0'; ds7_cs <= '0';
						cpu_data_in <= x"ff";
					when "010" =>
						ds0_cs <= '0'; ds1_cs <= '0'; ds2_cs <= cpu_vma; ds3_cs <= '0'; ds4_cs <= '0'; ds5_cs <= '0'; ds6_cs <= '0'; ds7_cs <= '0';
						cpu_data_in <= x"ff";
					when "011" =>
						ds0_cs <= '0'; ds1_cs <= '0'; ds2_cs <= '0'; ds3_cs <= cpu_vma; ds4_cs <= '0'; ds5_cs <= '0'; ds6_cs <= '0'; ds7_cs <= '0';
						cpu_data_in <= x"ff";
					when "100" =>
						ds0_cs <= '0'; ds1_cs <= '0'; ds2_cs <= '0'; ds3_cs <= '0'; ds4_cs <= cpu_vma; ds5_cs <= '0'; ds6_cs <= '0'; ds7_cs <= '0';
						cpu_data_in <= x"ff";
					when "101" =>
						ds0_cs <= '0'; ds1_cs <= '0'; ds2_cs <= '0'; ds3_cs <= '0'; ds4_cs <= '0'; ds5_cs <= cpu_vma; ds6_cs <= '0'; ds7_cs <= '0';
						cpu_data_in <= ds5_data_out;
					when "110" =>
						ds0_cs <= '0'; ds1_cs <= '0'; ds2_cs <= '0'; ds3_cs <= '0'; ds4_cs <= '0'; ds5_cs <= '0'; ds6_cs <= cpu_vma; ds7_cs <= '0';
						cpu_data_in <= x"ff";
					when "111" =>
						ds0_cs <= '0'; ds1_cs <= '0'; ds2_cs <= '0'; ds3_cs <= '0'; ds4_cs <= '0'; ds5_cs <= '0'; ds6_cs <= '0'; ds7_cs <= cpu_vma;
						cpu_data_in <= x"ff";
					end case;

					rombios_cs <= '0';
					ram_cs     <= '0';
				else													-- RAM
					cpu_data_in <= ram_data_out;
					rombios_cs  <= '0';
					ram_cs      <= cpu_vma;
					ds0_cs <= '0'; ds1_cs <= '0'; ds2_cs <= '0'; ds3_cs <= '0'; ds4_cs <= '0'; ds5_cs <= '0'; ds6_cs <= '0'; ds7_cs <= '0';
				end if;
			when others =>											-- RAM
				cpu_data_in <= ram_data_out;
				rombios_cs  <= '0';
				ram_cs      <= cpu_vma;
				ds0_cs <= '0'; ds1_cs <= '0'; ds2_cs <= '0'; ds3_cs <= '0'; ds4_cs <= '0'; ds5_cs <= '0'; ds6_cs <= '0'; ds7_cs <= '0';
		end case;
	end process;

	vramclock: process (clk, sys_clk, ram_cs, cpu_rw, cpu_addr, cpu_data_out, mux_ram_data_out)
	begin
		if sys_clk'event and sys_clk = '1' then
			if (rst = '0') then
				ram_state <= Idle;
				ram_hold <= '0';
--			if ((vram_access = '1') and (ram_cs = '0')) then
--				mux_ram_cs <= '1'; -- vram_cs;
--				mux_ram_rw <= '1'; -- vram_rw; -- read-only
--				mux_ram_addr <= vram_addr;
--				mux_ram_data_in <= vram_data_in;
			else
				mux_ram_cs <= ram_cs;
--				mux_ram_rw <= cpu_rw;
				mux_ram_addr <= cpu_addr;
				mux_ram_data_in <= cpu_data_out;
				ram_data_out <= mux_ram_data_out;
				case ram_state is
					when Idle =>
						if ((ram_cs = '1') and (cpu_rw = '1')) then
							ram_hold <= '1';
							ram_state <= Read;
						elsif ((ram_cs = '1') and (cpu_rw = '0')) then
							ram_hold <= '1';
							mux_ram_rw <= '0';
							ram_state <= Write;
--						else
--							ram_state <= Idle;
						end if;
					when Read =>
						ram_hold <= '0';
						ram_state <= Idle;
					when Write =>
						ram_hold <= '0';
						mux_ram_rw <= '1';
--						ram_state <= WrtEnd;
--					when WrtEnd =>
						ram_state <= Idle;
					when others =>
				end case;
			end if;
		end if;
	end process;	

	mc6845: process (cpu_addr, cpu_rw, cpu_data_out, ds0_cs, sys_clk)
	begin
		if (sys_clk'event and sys_clk = '1') then
			if (ds0_cs = '1') then
				if (cpu_rw = '0') then
					if (cpu_addr(0) = '0') then
						mc6845_addr <= cpu_data_out;
					else
						case (mc6845_addr(3 downto 0)) is
							when x"c"	=> vram_base_addr(15 downto 8) <= cpu_data_out;
							when x"d"	=> vram_base_addr( 7 downto 0) <= cpu_data_out;
							when others	=>	null;
						end case;
					end if;
				else
					if (cpu_addr(0) = '0') then
						mc6845_data_out <= mc6845_addr;
					else
						case (mc6845_addr(3 downto 0)) is
							when x"c"	=> mc6845_data_out <= vram_base_addr(15 downto 8);
							when x"d"	=> mc6845_data_out <= vram_base_addr( 7 downto 0);
							when others	=>	null;
						end case;
					end if;
				end if;
			end if;
		end if;
	end process;
	
	vram_cs <= video_en;
	video_mode <= '0';
--	vram_base_addr <= x"0000";
	
	vgafb: entity work.vgaframebuffer port map (
		rst		=> sys_rst,
		clk		=> clk25,
		pixel_clk=> pixel_clk,
		enable	=> video_en,
		mode		=> video_mode,
		addr_base=> vram_base_addr,
		addr_out	=> vram_addr,
		data_in	=> mux_ram_data_out,
		vga_r		=> vga_r,
		vga_g		=> vga_g,
		vga_b		=> vga_b,
		vga_hs	=> vga_hs,
		vga_vs	=> vga_vs
	);
	
end pyldin_arch;
