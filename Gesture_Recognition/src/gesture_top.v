module gesture_top (
    input clk,          // 27MHz
    input rst_n,        // S1 button
    
    // SPI Interface
    input spi_sclk,
    input spi_mosi,
    input spi_cs_n,
    
    // Indicators
    output [1:0] led    // D3, D4
);

    wire [7:0] rx_byte;
    wire rx_done;
    wire [479:0] imu_window;
    wire window_valid;
    wire [1:0] gesture_result;

    // SPI Slave Instance
    spi_slave spi_inst (
        .clk(clk),
        .rst_n(rst_n),
        .sclk(spi_sclk),
        .mosi(spi_mosi),
        .cs_n(spi_cs_n),
        .miso(),
        .rx_byte(rx_byte),
        .rx_done(rx_done)
    );

    // IMU Buffer Instance (80 samples * 6 bits = 480 bits)
    imu_buffer buffer_inst (
        .clk(clk),
        .rst_n(rst_n),
        .byte_in(rx_byte),
        .byte_ready(rx_done),
        .bnn_window(imu_window),
        .window_ready(window_valid)
    );

    // Gesture Classifier (1D-BCNN Brain)
    gesture_bcnn ai_core (
        .clk(clk),
        .rst_n(rst_n),
        .bnn_input(imu_window),
        .window_valid(window_valid),
        .gesture_out(gesture_result)
    );

    // Indicators: 
    // LED[0] and LED[1] now show the binary result of the gesture
    // 00: Idle, 01: Wave, 10: Shake, 11: Circle
    assign led = gesture_result;

endmodule
