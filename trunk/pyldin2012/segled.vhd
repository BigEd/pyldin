library ieee;
use ieee.std_logic_1164.all;
use ieee.std_logic_arith.all;
use ieee.std_logic_unsigned.all;

entity segleds is
port(
	clk					: in std_logic;
	rst					: in std_logic;
	ledseg				: out std_logic_vector(7 downto 0);
	ledcom				: out std_logic_vector(7 downto 0);
	data					: in std_logic_vector(31 downto 0)
	);
end segleds;

architecture segleds_arch of segleds is
signal div_cnt			: std_logic_vector(18 downto 0);
signal data4			: std_logic_vector(3 downto 0);
signal ledseg_xhdl	: std_logic_vector(7 downto 0);
signal ledcom_xhdl	: std_logic_vector(7 downto 0);
begin
	ledseg <= ledseg_xhdl;
	ledcom <= ledcom_xhdl;
	process(clk,rst)
	begin
		if(rst = '0')then 
			div_cnt <= "0000000000000000000";
		elsif (clk'event and clk = '1') then
			div_cnt <= div_cnt + 1;
		end if;
	end process;

	process(rst, clk, div_cnt(19 downto 17))
	begin
		if(rst = '0') then
			ledcom_xhdl <= "11111110";
		elsif (clk'event and clk = '1') then
			case div_cnt(18 downto 16) is
				when"000" => ledcom_xhdl <= "11111110";
				when"001" => ledcom_xhdl <= "11111101";
				when"010" => ledcom_xhdl <= "11111011";
				when"011" => ledcom_xhdl <= "11110111"; 
				when"100" => ledcom_xhdl <= "11101111";
				when"101" => ledcom_xhdl <= "11011111";
				when"110" => ledcom_xhdl <= "10111111";
				when"111" => ledcom_xhdl <= "01111111"; 
				when others => ledcom_xhdl <= "11111111";
			end case;
		end if;
	end process;

	process(ledcom_xhdl, data)
	begin
		case ledcom_xhdl is 
			when "11111110" => data4 <= data(31 downto 28);
			when "11111101" => data4 <= data(27 downto 24);
			when "11111011" => data4 <= data(23 downto 20);
			when "11110111" => data4 <= data(19 downto 16);
			when "11101111" => data4 <= data(15 downto 12);
			when "11011111" => data4 <= data(11 downto 8);
			when "10111111" => data4 <= data(7 downto 4);
			when "01111111" => data4 <= data(3 downto 0);
			when others     => data4 <= "0000";
		end case;
	end process;

	process(data4)
	begin
		case data4 is
			when "0000" => ledseg_xhdl <= "11000000";    
         when "0001" => ledseg_xhdl <= "11111001";    
         when "0010" => ledseg_xhdl <= "10100100";    
         when "0011" => ledseg_xhdl <= "10110000";    
         when "0100" => ledseg_xhdl <= "10011001";    
         when "0101" => ledseg_xhdl <= "10010010";    
         when "0110" => ledseg_xhdl <= "10000010";    
         when "0111" => ledseg_xhdl <= "11111000";    
         when "1000" => ledseg_xhdl <= "10000000";    
         when "1001" => ledseg_xhdl <= "10010000";    
         when "1010" => ledseg_xhdl <= "10001000";    
         when "1011" => ledseg_xhdl <= "10000011";    
         when "1100" => ledseg_xhdl <= "11000110";    
         when "1101" => ledseg_xhdl <= "10100001";    
         when "1110" => ledseg_xhdl <= "10000110";    
         when "1111" => ledseg_xhdl <= "10001110";    
         when others => ledseg_xhdl <= "00000011"; 
		end case;
	end process;
end segleds_arch;
