	
	module nios_sys_wrapper (
		////////////////////	Audio CODEC		////////////////////////////
		inout			AUD_ADCLRCK,			//	Audio CODEC ADC LR Clock
		input			AUD_ADCDAT,				//	Audio CODEC ADC Data
		inout			AUD_DACLRCK,			//	Audio CODEC DAC LR Clock
		output			AUD_DACDAT,				//	Audio CODEC DAC Data
		inout			AUD_BCLK,				//	Audio CODEC Bit-Stream Clock
		output			AUD_XCK,				//	Audio CODEC Chip Clock
		
		inout			I2C_SDAT,				//	I2C Data
		output		I2C_SCLK,				//	I2C Clock
		
		input  wire        CLK_50,                           		//          clk_50.clk
		input  wire			 reset_n,
		input  wire	[2:0]	 SW,
		
		input  wire [3:0]  external_key,                     //    external_key.key
		output wire [8:0]  external_led,                     //    external_led.led
		
		output wire [7:0] VGA_R,
		output wire [7:0] VGA_G,
		output wire [7:0] VGA_B,
		output wire VGA_HS,
		output wire	VGA_VS,
		output wire VGA_SYNC_N,
		output wire VGA_BLANK_N,
		output wire VGA_CLK,
		
		output [19:0] SRAM_ADDR,
		output [1:0] SRAM_BE_N,
		output SRAM_OE_N,
		output SRAM_WE_N,
		output SRAM_CE_N,
		inout [15:0] SRAM_DQ
	);
	
	
	wire rst_n;
	assign rst_n = 1'b1;
	
	wire [9:0] vga_r10,vga_g10,vga_b10;
	assign VGA_R = vga_r10[9:2];
	assign VGA_G = vga_g10[9:2];
	assign VGA_B = vga_b10[9:2];

	reg clk25;	
	wire clk27;
	wire clk100;
	
	Audio_PLL2 p2 (
	.areset(!reset_n),
	.inclk0(CLK_50),
	.c0(clk27)
	);
	
always @(posedge CLK_50 or negedge reset_n)
begin
    if(!reset_n)
        clk25   <=  1'b0        ;
    else
        clk25   <=  ~clk25  ;     
end
	
		//connect to nios_sys_inst
	//add five pio in nios sys and allocate address
	wire   [18:0]   export_vga_pio_pixel_pos;
	wire   [9:0]    export_vga_pio_r_on;
	wire   [9:0]    export_vga_pio_g_on;	
	wire   [9:0]    export_vga_pio_b_on;
	wire   [9:0]    export_vga_pio_r_off;
	wire   [9:0]    export_vga_pio_g_off;	
	wire   [9:0]    export_vga_pio_b_off;
	wire	 export_vga_pio_pos_state;
	
VGA_NIOS_CTRL vga_controller_inst(

	.clk(clk100),
	.rst_n(reset_n) ,
	.pixel_pos(export_vga_pio_pixel_pos),
	.pos_state(export_vga_pio_pos_state), // pos (x,y)'s state
	.red_on(export_vga_pio_r_on)   , // on state r value
	.green_on(export_vga_pio_g_on) , // on state g value
	.blue_on(export_vga_pio_b_on)  , // on state b value
	.red_off(export_vga_pio_r_off)   , // off state r value
	.green_off(export_vga_pio_g_off) , // off state g value
	.blue_off(export_vga_pio_b_off)  , // off state b value

						//	Export Side
	.avs_s1_export_VGA_R(vga_r10),
	.avs_s1_export_VGA_G(vga_g10),
	.avs_s1_export_VGA_B(vga_b10),
	.avs_s1_export_VGA_HS(VGA_HS),
	.avs_s1_export_VGA_VS(VGA_VS),
	.avs_s1_export_VGA_SYNC(VGA_SYNC_N),
	.avs_s1_export_VGA_BLANK(VGA_BLANK_N),
	.avs_s1_export_VGA_CLK(VGA_CLK),
	.avs_s1_export_iCLK_25(clk25)
	);
	
	
	
	nios_cpu nios_sys_inst (
		.audio_clk_50_clk							  (CLK_50), 
		.audio_io_adc_data                    (AUD_ADCDAT),                    //        audio_io.adc_data
		.audio_io_adc_clk                     (AUD_ADCLRCK),                     //                .adc_clk
		.audio_io_bit_stream_clk              (AUD_BCLK),              //                .bit_stream_clk
		.audio_io_dac_data                    (AUD_DACDAT),                    //                .dac_data
		.audio_io_dac_clk                     (AUD_DACLRCK),                     //                .dac_clk
		.audio_io_chip_clk                    (AUD_XCK),                    //                .chip_clk
		
		.clk_50_clk                      	  (CLK_50),                           //          clk_50.clk
		.external_key_key                     (external_key),                     //    external_key.key
		.external_led_led                     (external_led),                     //    external_led.led
		
		.reset_reset_n                        (rst_n),                        //           reset.reset_n
		
		.sram_bridge_out_tcm_address_out      (SRAM_ADDR),      // sram_bridge_out.tcm_address_out
		.sram_bridge_out_tcm_byteenable_n_out (SRAM_BE_N), //                .tcm_byteenable_n_out
		.sram_bridge_out_tcm_read_n_out       (SRAM_OE_N),       //                .tcm_read_n_out
		.sram_bridge_out_tcm_write_n_out      (SRAM_WE_N),      //                .tcm_write_n_out
		.sram_bridge_out_tcm_data_out         (SRAM_DQ),         //                .tcm_data_out
		.sram_bridge_out_tcm_chipselect_n_out (SRAM_CE_N), //                .tcm_chipselect_n_out
		

		
		.clk_27_clk                    (clk27),
		.external_i2c_i2c_sclk         (I2C_SCLK),      //    external_i2c.i2c_sclk
		.external_i2c_i2c_sdat         (I2C_SDAT),       //                .i2c_sdat
		
		.clk_25_clk							 (),
		.clk_100_clk                   (clk100),  
	
		.external_sw_export            (SW[2:0]),

		.external_pio_r_on_export                (export_vga_pio_r_on),                //  external_pio_r.export
		.external_pio_g_on_export                (export_vga_pio_g_on),                //  external_pio_g.export
		.external_pio_b_on_export                (export_vga_pio_b_on),                //  external_pio_b.export
		.external_pio_r_off_export            (export_vga_pio_r_off),            //     external_pio_r_off.export
		.external_pio_g_off_export            (export_vga_pio_g_off),            //     external_pio_g_off.export
		.external_pio_b_off_export            (export_vga_pio_b_off),            //     external_pio_b_off.export
		.external_pio_pos_state_export        (export_vga_pio_pos_state),         // external_pio_pos_state.export
		.external_pio_pixel_pos_export        (export_vga_pio_pixel_pos)		
	);
	
	
	endmodule

