  /*
  @Func: encapsulate the audio process module with avalon memory mappped interface.
  
			Register Map:
			00 -> data write fft_addr_reg register, data read fft_done_reg register
			01 -> data write fft_start_reg register, data read fft_power_reg register
			10 -> data read fft_exp_reg register
			11 -> reserved
  @Author: xjy
  @Date: 2018/12/6
  */
  
  module audio_proc_avalon_mm(
  
		//Avalon Memory Mapped Slave Interface
		input wire clk,
		input wire reset_n,
      input wire [1:0] address,
      input wire [31:0] writedata,
      input wire [3:0] byteenable,
      input wire write,
      input wire read,
      input wire chipselect,
      output reg [31:0] readdata,
		
		//External Ports
		////////////////////////	Clock Input	 	////////////////////////
		input			CLOCK_27,				//	27 MHz
		input			CLOCK_50,				//	50 MHz
		////////////////////	Audio CODEC		////////////////////////////
		inout			AUD_ADCLRCK,			//	Audio CODEC ADC LR Clock
		input			AUD_ADCDAT,				//	Audio CODEC ADC Data
		inout			AUD_DACLRCK,			//	Audio CODEC DAC LR Clock
		output			AUD_DACDAT,				//	Audio CODEC DAC Data
		inout			AUD_BCLK,				//	Audio CODEC Bit-Stream Clock
		output			AUD_XCK,				//	Audio CODEC Chip Clock
		////////////////////////	I2C		////////////////////////////////
		inout			I2C_SDAT,				//	I2C Data
		output		I2C_SCLK,				//	I2C Clock
		////////////////////////	Push Button		////////////////////////
		input	[3:0]	KEY,					//	Pushbutton[3:0]
		////////////////////////////	LED		////////////////////////////
		output	[8:0]	LEDG 			 //	LED Green[8:0]
		
	);
	

	reg [15:0] fft_addr_reg;
	reg [7:0] fft_start_reg;
	reg [7:0] fft_done_reg;
	reg [15:0] fft_power_reg;
	reg [7:0] fft_exp_reg;
	
	//write select signals
	reg fft_addr_reg_sel;
	reg fft_start_reg_sel;
	
	
	//address decoding
	always@ (address) begin
		fft_addr_reg_sel <= 'b0;
		fft_start_reg_sel <= 'b0;
		
		case(address)
			2'b00: fft_addr_reg_sel <= 1;
			2'b01: fft_start_reg_sel <= 1;
			default: begin
				fft_addr_reg_sel <= 0;
				fft_start_reg_sel <= 0;
			end
		endcase
		
	end
	
	
	//write register fft_addr_reg
	always@ (posedge clk or negedge reset_n) begin
		if(!reset_n)
			fft_addr_reg <= 0;
		else begin
			if(write & chipselect & fft_addr_reg_sel) begin
				if(byteenable[0])
					fft_addr_reg[7:0] <= writedata[7:0];
				if(byteenable[1])
					fft_addr_reg[15:8] <= writedata[15:8];
			end		
		end
	end
	
	//write register fft_start_reg
	always@ (posedge clk or negedge reset_n) begin
		if(!reset_n)
			fft_start_reg <= 0;
		else begin
			if(write & chipselect & fft_start_reg_sel) begin
				if(byteenable[0])
					fft_start_reg[7:0] <= writedata[7:0];
			end		
		end
	end

	//read registers
	always@ (address or read or fft_done_reg or fft_power_reg or fft_exp_reg or chipselect) begin
		if(read & chipselect)
			case(address)
				2'b00: readdata <= fft_done_reg;
				2'b01: readdata <= fft_power_reg;
				2'b10: readdata <= fft_exp_reg;
				default: readdata <= 0;
			endcase
	end
	
	wire fftdone;
	wire [15:0] fftpower;
	wire [5:0] fftexp;
	
	always@ (posedge clk or negedge reset_n) begin
		if(!reset_n) begin
			fft_done_reg <= 0;
			fft_power_reg <= 0;
			fft_exp_reg <= 0;
		end
		else begin
			fft_done_reg <= fftdone;
			fft_power_reg <= fftpower;
			fft_exp_reg <= fftexp;
		end
	end

	//audio process module
	audio_proc_top audio_proc_inst(
	////////////////////////	Clock Input	 	////////////////////////
			.CLOCK_27(CLOCK_27),				//	27 MHz
			.CLOCK_50(CLOCK_50),				//	50 MHz
	////////////////////////	Push Button		////////////////////////
			.KEY(KEY),					//	Pushbutton[3:0]
	////////////////////////////	LED		////////////////////////////
			.LEDG(LEDG),					//	LED Green[8:0]
	////////////////////	Audio CODEC		////////////////////////////
			.AUD_ADCLRCK(AUD_ADCLRCK),			//	Audio CODEC ADC LR Clock
			.AUD_ADCDAT(AUD_ADCDAT),				//	Audio CODEC ADC Data
			.AUD_DACLRCK(AUD_DACLRCK),			//	Audio CODEC DAC LR Clock
			.AUD_DACDAT(AUD_DACDAT),				//	Audio CODEC DAC Data
			.AUD_BCLK(AUD_BCLK),				//	Audio CODEC Bit-Stream Clock
			.AUD_XCK(AUD_XCK),				//	Audio CODEC Chip Clock
	////////////////////////	I2C		////////////////////////////////
			.I2C_SDAT(I2C_SDAT),				//	I2C Data
			.I2C_SCLK(I2C_SCLK),				//	I2C Clock
	////////////////////	NIOS INTERFACE		////////////////////////////
			.NIOS_CLK(CLOCK_50),
			.fftaddr(fft_addr_reg[9:0]),
			.fftstart(fft_start_reg[0]),
			.fftdone(fftdone),
			.fftpower(fftpower),
			.fftexp(fftexp)	
	);
	
	endmodule
		
  
 