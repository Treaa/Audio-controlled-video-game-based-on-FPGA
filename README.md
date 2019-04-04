# Audio-controlled-video-game-based-on-FPGA
This is a spaceship video game controlled by user's audio and implemented on Altera DE2-115 FPGA board.


# Main idea
This project designs and implements real-time audio acquisition, audio processing, the related VGA control logic of the graphic display on the FPGA, and implements the relevant spaceship game algorithm on the embedded NIOS II cpu. The corresponding interactive interface for communication between hardware and software should be concerned.


# System Architecture
* The user inputs the audio through the microphone and the MIC interface on the DE2-115. Then we configure the AUDIO CODEC chip on the FPGA board by controlling the I2C bus inside the DE2-115. The AUDIO CODEC chip is set at a certain sampling rate and sampling points, and the audio samples are made for analog-to-digital conversion.
* When the control logic of the dual port AUDIO RAM receives the audio information read request `FFT_START` from the NIOS II, the audio information processed by AUDIO_CODEC is firstly written into the AUDIO_RAM, then FFT_Controller read it into the FFT IP to perform FFT transformation.
* After the transformation is completed, the obtained multi-point FFT data is stored in the dual-port FFT_RAM, and then the FFT controller generates a flag signal `FFT_DONE` and writes it back to NIOS II.
* After receiving the `FFT_DONE` signal, NIOS II will generate a series of RAM address `FFT_ADDR` and then read the FFT-transformed audio information from FFT_RAM. 
* The audio data acquired and processed by FPGA in real time is used as the input information of the audio control game, and the software game algorithm continuously generates the real-time screen information. These information will be transmitted to the VGA_Controller module on the FPGA through PIO port, and the real-time game information will be displayed on the monitor through the VGA interface.
