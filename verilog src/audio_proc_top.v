
module audio_proc_top (
	////////////////////////	Clock Input	 	////////////////////////
	input			CLOCK_27,				//	27 MHz
	input			CLOCK_50,				//	50 MHz
	////////////////////////	Push Button		////////////////////////
	input	[3:0]	KEY,					//	Pushbutton[3:0]
	////////////////////////////	LED		////////////////////////////
	output	[8:0]	LEDG,					//	LED Green[8:0]
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
	////////////////////	NIOS INTERFACE		////////////////////////////
	input wire			NIOS_CLK,
	input wire [9:0]  fftaddr,
	input wire        fftstart,
	output wire        fftdone,
	output wire [15:0] fftpower,
	output wire [5:0]  fftexp	
);


/// audio stuff /////////////////////////////////////////////////
// output to audio DAC
wire signed [15:0] audio_outL, audio_outR;
// input from audio ADC
wire signed [15:0] audio_inL, audio_inR;


wire	AUD_CTRL_CLK;
wire	DLY_RST;
wire audio_ram_wr_clk;

assign	AUD_ADCLRCK = audio_ram_wr_clk;
assign	AUD_XCK = AUD_CTRL_CLK;
assign	AUD_DACLRCK = audio_ram_wr_clk;

Reset_Delay inst0 (
	.iCLK(CLOCK_50),
	.oRESET(DLY_RST)
);

Audio_PLL p1 (
	.areset(~DLY_RST),
	.inclk0(CLOCK_27),
	.c0(AUD_CTRL_CLK)
);


I2C_AV_Config I2C_inst (
	// Host Side
	.iCLK(CLOCK_50),
	.iRST_N(KEY[0]),
	//.o_I2C_END(I2C_END),
	// I2C Side
	.I2C_SCLK(I2C_SCLK),
	.I2C_SDAT(I2C_SDAT)
);

AUDIO_DAC_ADC dac_inst (
	// Audio Side
	.oAUD_BCK(AUD_BCLK),
	.oAUD_DATA(AUD_DACDAT),
	.oAUD_LRCK(audio_ram_wr_clk),
	.oAUD_inL(audio_inL), // audio data from ADC 
	.oAUD_inR(audio_inR), // audio data from ADC 
	.iAUD_ADCDAT(AUD_ADCDAT),
	.iAUD_extL(audio_outL), // audio data to DAC
	.iAUD_extR(audio_outR), // audio data to DAC
	// Control Signals
	.iCLK_18_4(AUD_CTRL_CLK),
	.iRST_N(DLY_RST)
);


// FFT controller stuff
parameter LEN   = 1024;
parameter LBITS = 10;
parameter EBITS = 6;
parameter BITS  = 16;

wire lc;
wire [LBITS-1:0] sampleAddr;
wire [BITS-1:0] sample;

FFTController fftc_inst (
	.iReset(~KEY[0]),
	.iStart(lc),
	.iStateClk(NIOS_CLK),
	.oSampAddr(sampleAddr),
	.iSamp(sample),
	.iReadAddr(fftaddr),
	.iReadClock(NIOS_CLK),
	.oPower(fftpower),
	.oExp(fftexp),
	.oDone(fftdone)
);

AudioRam_controller audRAMc_inst (
	.iReset(~KEY[0]),
	.iStartLoad(fftstart),
	.iWriteClock(audio_ram_wr_clk),
	.iSample(audio_inL),
	.iReadClock(NIOS_CLK),
	.iReadAddr(sampleAddr),
	.iWindow(KEY[1]),
	.oValue(sample),
	.oLoadComplete(lc)
);

assign LEDG[8] = fftstart;

endmodule