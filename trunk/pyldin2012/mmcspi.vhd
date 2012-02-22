--
-- MMC/SD SPI mode controller
-- ADDR	DIR	FUNC
--  00	rw		read received byte, write any byte to start receving
--	 01	 w		write byte to start transmittion
--	 10	rw		bit 7 = 1 if rx/tx operation finished, bit 0 - enable/disable card (1/0)
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
signal ctrl					: std_logic_vector(7 downto 0);
begin
	mmc_cs <= ctrl(0);
	mmc_ck <= ctrl(1);
	mmc_di <= ctrl(2);
	
	ctrlogic: process(clk)
	begin
		if (clk'event and clk = '1') then
			if (rst = '1') then
				ctrl		<= x"00";
			elsif (cs = '1') then
				if (rw = '0') then
					if (address = "10") then
						ctrl		<= data_in;
					end if;
				else
					if (address = "10") then
						data_out(6 downto 0) <= ctrl(6 downto 0);
						data_out(7) <= mmc_do;
					end if;
				end if;
			end if;
		end if;
	end process;
	
end mmcspi_arch;
