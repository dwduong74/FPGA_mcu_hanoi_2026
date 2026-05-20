`timescale 1ns / 1ps

module gesture_tb();

    // 1. Signals
    reg clk;
    reg rst_n;
    reg spi_sclk;
    reg spi_mosi;
    reg spi_cs_n;
    wire [1:0] led;

    // 2. Instantiate Top Module
    gesture_top uut (
        .clk(clk),
        .rst_n(rst_n),
        .spi_sclk(spi_sclk),
        .spi_mosi(spi_mosi),
        .spi_cs_n(spi_cs_n),
        .led(led)
    );

    // 3. Clock Generation (27MHz ~ 37ns period)
    always #18.5 clk = ~clk;

    // 4. SPI Master Task (Simulates MCU)
    task send_spi_byte(input [7:0] data);
        integer i;
        begin
            for (i = 7; i >= 0; i = i - 1) begin
                spi_mosi = data[i];
                #100; // Half SCLK period
                spi_sclk = 1;
                #100; // Half SCLK period
                spi_sclk = 0;
            end
            #500; // Gap between bytes
        end
    endtask

    task send_imu_sample(input [7:0] ax, input [7:0] ay, input [7:0] az, 
                         input [7:0] gx, input [7:0] gy, input [7:0] gz);
        begin
            spi_cs_n = 0;
            send_spi_byte(ax);
            send_spi_byte(ay);
            send_spi_byte(az);
            send_spi_byte(gx);
            send_spi_byte(gy);
            send_spi_byte(gz);
            spi_cs_n = 1;
            #1000; // Delay between samples
        end
    endtask

    // 5. Stimulus
    initial begin
        // Initialize
        clk = 0;
        rst_n = 0;
        spi_sclk = 0;
        spi_mosi = 0;
        spi_cs_n = 1;

        // Reset system
        #100;
        rst_n = 1;
        #100;

        $display("Starting Simulation: Sending 80 samples...");

        // Send 80 samples to fill the buffer
        // Let's send a pattern that looks like a "Wave" (Sign bits alternating)
        repeat (80) begin
            send_imu_sample(8'hFF, 8'h00, 8'hFF, 8'h00, 8'hFF, 8'h00);
        end

        $display("Buffer primed. Waiting for BCNN inference...");
        
        // Wait for BCNN to process (approx 500-1000 clock cycles)
        #50000;

        $display("Simulation finished. Final LED state: %b", led);
        $finish;
    end

endmodule
