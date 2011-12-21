--
-- MMC/SD SPI mode controller
-- ADDR	DIR	FUNC
--  00	r		read received byte
--	 01	 w		write byte to start transmittion
--	 10	r		bit 7 = 1 if rx/tx operation finished, bit 0 - enable/disable card (1/0)
--	 10	 w		set bit 7 = 1 to start receiving byte
--
library ieee;
use ieee.std_logic_1164.all;
use ieee.std_logic_arith.all;
use ieee.std_logic_unsigned.all;

entity mmcspi is
port (
	rst						: in	std_logic;
	clk						: in	std_logic;

	cs							: in	std_logic;
	rw							: in	std_logic;
	address					: in	std_logic_vector(1 downto 0);
	data_in					: in	std_logic_vector(7 downto 0);
	data_out					: out	std_logic_vector(7 downto 0);

	mmc_cs					: out std_logic;
	mmc_ck					: out std_logic;
	mmc_di					: out std_logic;
	mmc_do					: in  std_logic
);
end mmcspi;

architecture mmcspi_arch of mmcspi is
signal txdat				: std_logic_vector(7 downto 0);
signal iodat				: std_logic_vector(7 downto 0);
signal ctrl					: std_logic_vector(7 downto 0);
signal rx_start			: std_logic;
signal tx_start			: std_logic;
signal io_cycle			: std_logic_vector(3 downto 0);
type clk_states is (Idle, High, Low);
signal io_clk_state		: clk_states;
begin
	mmc_cs <= not ctrl(0);

	ctrlogic: process(clk)
	begin
		if (clk'event and clk = '1') then
			if (rst = '1') then
				ctrl		<= x"00";
				txdat		<= x"00";
				rx_start <= '0';
				tx_start <= '0';
			elsif (cs = '1') then
				if (rw = '0') then
					if (address = "01") then
						txdat		<= data_in;
						tx_start	<= '1';
					elsif (address = "10") then
						ctrl		<= data_in;
						rx_start	<= data_in(7);
					end if;
				else
					if (address = "00") then
						data_out <= iodat;
					elsif (address = "10") then
						data_out(6 downto 0) <= ctrl(6 downto 0);
						if (((tx_start or rx_start) = '1') and (io_cycle = "1000")) then
							data_out(7)	<= '1';
							tx_start		<= '0';
							rx_start		<= '0';
						else
							data_out(7) <= '0';
						end if;
					end if;
				end if;
			end if;
		end if;
	end process;
	
	rxtxlogic: process(clk)
	begin
		if (clk'event and clk = '1') then
			if (rst = '1') then
				io_cycle <= "1111";
				io_clk_state <= Idle;
			elsif (((tx_start or rx_start) = '0') and (io_cycle = "1000")) then
				io_cycle <= "1111";
			elsif (((tx_start or rx_start) = '1') and (io_cycle = "1111")) then
				io_cycle <= "0000";
				mmc_di <= rx_start; -- send 1 to DI if receive data
				iodat <= txdat;
			elsif (io_clk_state /= Idle) then
				if (io_clk_state = High) then
					mmc_ck <= '1';
					io_clk_state <= Low;
				elsif (io_clk_state = Low) then
					mmc_ck <= '0';
					io_clk_state <= Idle;
					if (io_cycle /= "1000") then
						iodat(7 downto 1) <= iodat(6 downto 0);
					end if;
				end if;
			elsif ((io_cycle /= "1000") and (io_cycle /= "1111")) then
				if (tx_start = '1') then
					mmc_di <= iodat(7);
				else
					iodat(0) <= mmc_do;
				end if;
				io_cycle <= io_cycle + 1;
				io_clk_state <= High;
			end if;
		end if;
	end process;
end mmcspi_arch;
