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
signal clk25				: std_logic;
signal sys_clk				: std_logic;

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
signal ram_wr				: std_logic; -- memory write enable
signal ram_oe				: std_logic; -- memory output enable
signal ram_cs          	: std_logic; -- memory chip select
signal ram_data_out    	: std_logic_vector(7 downto 0);

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
signal video_clk25		: std_logic;
signal video_row			: std_logic_vector(9 downto 0);
signal video_column		: std_logic_vector(9 downto 0);
signal video_en			: std_logic;
signal video_r				: std_logic_vector(2 downto 0);
signal video_g				: std_logic_vector(2 downto 0);
signal video_b				: std_logic_vector(1 downto 0);

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

	cpuclock: process (clk)
	begin
		if clk'event and clk='1' then
			if (clk25 = '0')then
				clk25 <= '1';
			else
				clk25 <= '0';
			end if;
		end if;
	end process;	

	stepone: process(clk, step)
	begin
		if (clk'event and clk = '1') then
			if (step = '0') then
				if (step_debouncer = "0010000000000000000000000") then
					step_clk <= '1';
					step_debouncer <= step_debouncer + 1;
				elsif (step_debouncer = "1100000000000000000000000") then
					step_debouncer <= "0000000000000000000000000";
					step_clk <= '0';
				else
					step_debouncer <= step_debouncer + 1;
				end if;
			else
				step_debouncer <= "0000000000000000000000000";
				step_clk <= '0';
			end if;
		end if;
	end process;

	videosync: entity work.vgasync port map(
		clk		=> clk,
		vga_clk25=> video_clk25,
		vga_hs	=> vga_hs,
		vga_vs	=> vga_vs,
		row		=> video_row,
		column	=> video_column,
		enable	=>	video_en
	);
	
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
		cs			=> rombios_cs,
		addr		=> cpu_addr(11 downto 0),
		data		=> rombios_data_out
	);

	segdisplay : entity work.segleds port map(
		clk		=> clk,
		rst		=> rst,
		ledseg	=> ledseg,
		ledcom	=> ledcom,
		data		=> led_data
	);

	debugmode: process(swt, ds5_data_in, step_display, step_clk, clk25)
	begin
		if (swt = '0') then
			led_data <= ds5_data_in;
			sys_clk  <= clk25;
		else
			led_data <= step_display;
			sys_clk  <= step_clk;
		end if;
	end process;
	
	segdisptrace : process (cpu_addr, cpu_rw, cpu_data_in, cpu_data_out, swt, led_data)
	begin
		step_display(31 downto 16) <= cpu_addr;
		step_display(15 downto 8 ) <= x"00";
		if (cpu_rw = '1') then
			step_display(7 downto 0) <= cpu_data_in;
		else
			step_display(7 downto 0) <= cpu_data_out;
		end if;
	end process;

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
--				ds_cs		   <= '0';
				ds0_cs <= '0'; ds1_cs <= '0'; ds2_cs <= '0'; ds3_cs <= '0'; ds4_cs <= '0'; ds5_cs <= '0'; ds6_cs <= '0'; ds7_cs <= '0';
			when "1110" =>											-- $Exxx
				if (cpu_addr(11 downto 8) = "0110") then	-- IO $E6Xxx selector
					case (cpu_addr(7 downto 5)) is
					when "000" =>
						ds0_cs <= cpu_vma; ds1_cs <= '0'; ds2_cs <= '0'; ds3_cs <= '0'; ds4_cs <= '0'; ds5_cs <= '0'; ds6_cs <= '0'; ds7_cs <= '0';
						cpu_data_in <= x"ff";
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
--					ds_cs       <= '0';
					ds0_cs <= '0'; ds1_cs <= '0'; ds2_cs <= '0'; ds3_cs <= '0'; ds4_cs <= '0'; ds5_cs <= '0'; ds6_cs <= '0'; ds7_cs <= '0';
				end if;
			when others =>											-- RAM
				cpu_data_in <= ram_data_out;
				rombios_cs  <= '0';
				ram_cs      <= cpu_vma;
--				ds_cs       <= '0';
				ds0_cs <= '0'; ds1_cs <= '0'; ds2_cs <= '0'; ds3_cs <= '0'; ds4_cs <= '0'; ds5_cs <= '0'; ds6_cs <= '0'; ds7_cs <= '0';
		end case;
	end process;
	
	sram: process( sys_clk, rst, cpu_addr, cpu_rw, cpu_data_out,
                  ram_cs, ram_wr, ram_data_out, sram_dq )
	begin
--		led_data(8 ) <= '0'; -- not(ram_cs and rst); -- ce
--		led_data(9 ) <= not(cpu_rw and ram_cs and rst); -- oe
--		led_data(10) <= not(ram_cs and (not cpu_rw)); -- wr
--		led_data(11) <= '0';
--		led_data(12) <= '0'; -- cpu_addr(0) and (not cpu_rw) and sys_clk; -- lb
--		led_data(13) <= '0'; -- (not cpu_addr(0)) and (not cpu_rw) and sys_clk; -- ub
--		led_data(14) <= '0';
--		led_data(15) <= '0';
		
		sram_ce_n <= not(ram_cs and rst); -- put '0' to enable chip all time (no powersave mode)
		sram_oe_n <= not(cpu_rw and ram_cs and rst);
		ram_wr    <= not(ram_cs and (not cpu_rw) and sys_clk);
		sram_we_n <= ram_wr;
		sram_lb_n <= not cpu_addr(0);
		sram_ub_n <= cpu_addr(0);
		sram_addr(17 downto 16) <= "00";
		sram_addr(15 downto 0 ) <= cpu_addr(15 downto 0);

		if (ram_wr = '0' and cpu_addr(0) = '0') then
			sram_dq(15 downto 8) <= cpu_data_out;
		else
			sram_dq(15 downto 8)  <= "ZZZZZZZZ";
		end if;

		if (ram_wr = '0' and cpu_addr(0) = '1') then
			sram_dq(7 downto 0) <= cpu_data_out;
		else
			sram_dq(7 downto 0)  <= "ZZZZZZZZ";
		end if;

		if (cpu_addr(0) = '0') then
			ram_data_out <= sram_dq(15 downto 8);
		else
			ram_data_out <= sram_dq(7 downto 0);
		end if;
	end process;

	interrupts : process( rst )
	begin
		cpu_halt  <= '0';
		cpu_hold  <= '0';
		cpu_irq   <= '0'; -- uart_irq or timer_irq;
		cpu_nmi   <= '0'; -- trap_irq;
		cpu_reset <= not rst; -- CPU reset is active high
	end process;

	videoout: process(video_clk25, video_en, video_row, video_column, video_r, video_g, video_b)
	begin
		if (video_clk25'event and video_clk25 = '1') then
			if ((video_row >= 0) and (video_row <= 479)) then
				video_r <= "010";
				video_g <= "010";
				video_b <= "01";
			end if;
		end if;
		if (video_en = '1') then
			vga_r <= video_r;
			vga_g <= video_g;
			vga_b <= video_b;
		else
			vga_r <= "000";
			vga_g <= "000";
			vga_b <= "00";
		end if;
	end process;
	
end pyldin_arch;
