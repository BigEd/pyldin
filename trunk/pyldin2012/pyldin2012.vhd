library ieee;
use ieee.std_logic_1164.all;
use ieee.std_logic_arith.all;
use ieee.std_logic_unsigned.all;

entity pyldin2012 is
port(
	clk						: in std_logic;
	rst						: in std_logic;

--	vga_r               	: out std_logic_vector(2 downto 0);
--	vga_g               	: out std_logic_vector(2 downto 0);
--	vga_b               	: out std_logic_vector(1 downto 0);
--	vga_hs              	: out std_logic;
--	vga_vs              	: out std_logic;
		
	sram_addr           	: out   std_logic_vector(17 downto 0);
	sram_dq             	: inout std_logic_vector(15 downto 0);
	sram_ce_n           	: out   std_logic;
	sram_oe_n           	: out   std_logic;
	sram_we_n           	: out   std_logic;
	sram_ub_n           	: out   std_logic;
	sram_lb_n           	: out   std_logic;
	
	step						: in std_logic;
	ledseg					: out std_logic_vector(7 downto 0);
	ledcom					: out std_logic_vector(7 downto 0)
--	keys						: in std_logic_vector(2 downto 0)
);
end pyldin2012;

architecture pyldin_arch of pyldin2012 is
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

-- hardware debugger
signal led_data			: std_logic_vector(31 downto 0);
signal step_debouncer	: std_logic_vector(24 downto 0);
begin
	stepone: process(clk, step)
	begin
		if (clk'event and clk = '1') then
			if (step = '0') then
				if (step_debouncer = "0010000000000000000000000") then
					sys_clk <= '1';
					step_debouncer <= step_debouncer + 1;
				elsif (step_debouncer = "1100000000000000000000000") then
					step_debouncer <= "0000000000000000000000000";
					sys_clk <= '0';
				else
					step_debouncer <= step_debouncer + 1;
				end if;
			else
				step_debouncer <= "0000000000000000000000000";
				sys_clk <= '0';
			end if;
		end if;
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

	segdisptrace : process (cpu_addr, cpu_rw, cpu_data_in, cpu_data_out)
	begin
		led_data(31 downto 16) <= cpu_addr;
		led_data(15 downto 8 ) <= x"00";
		if (cpu_rw = '1') then
			led_data(7 downto 0) <= cpu_data_in;
		else
			led_data(7 downto 0) <= cpu_data_out;
		end if;
	end process;
	
	decode: process( cpu_addr, cpu_rw, cpu_vma, cpu_data_in,
					  rombios_cs, rombios_data_out,
				     ram_cs, ram_data_out
--				     uart_cs, uart_data_out,
--				     cf_cs, cf_data_out,
--				     timer_cs, timer_data_out,
					  )
	begin
      case cpu_addr(15 downto 12) is
			when "1111" => -- $F000
				cpu_data_in <= rombios_data_out;            -- read ROM
				rombios_cs <= cpu_vma;
				ram_cs     <= '0';
--				uart_cs    <= '0';
--				cf_cs      <= '0';
--				timer_cs   <= '0';
--				ioport_cs  <= '0';
			when others =>
				cpu_data_in <= ram_data_out;
				rombios_cs <= '0';
				ram_cs     <= cpu_vma;
--				uart_cs    <= '0';
--				cf_cs      <= '0';
--				timer_cs   <= '0';
--				ioport_cs  <= '0';
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

end pyldin_arch;
