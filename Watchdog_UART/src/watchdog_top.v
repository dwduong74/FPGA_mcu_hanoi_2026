module watchdog_top (
    input clk,          // Pin 4
    input rst_n,        // Pin 35 (S1)
    
    input wdi,          // Pin 36 (S2) - Watchdog Kick
    input uart_rx_pin,  // Pin 33
    
    output wdo_led,     // Pin 28 (D4) - Watchdog Output indicator
    output wdo_pin,     // Pin 29 (Header) - Watchdog Output
    output enout_led    // Pin 27 (D3) - Enable status indicator
);

    wire [7:0] rx_data;
    wire rx_done;
    wire [31:0] twd, trst, arm_dly;
    wire en_internal;
    wire wdo_internal;
    wire enout_internal;

    // UART RX module
    uart_rx #(
        .CLK_FREQ(27_000_000),
        .BAUD_RATE(9600)
    ) uart_rx_inst (
        .clk(clk),
        .rst_n(rst_n),
        .rx(uart_rx_pin),
        .rx_data(rx_data),
        .rx_done(rx_done)
    );

    // Configuration registers
    config_reg config_reg_inst (
        .clk(clk),
        .rst_n(rst_n),
        .rx_data(rx_data),
        .rx_done(rx_done),
        .twd_limit(twd),
        .trst_limit(trst),
        .arm_delay_limit(arm_dly),
        .en(en_internal)
    );

    // Watchdog Core logic
    watchdog_core watchdog_core_inst (
        .clk(clk),
        .rst_n(rst_n),
        .en(en_internal),
        .wdi(wdi),
        .wdo(wdo_internal),
        .enout(enout_internal),
        .twd_limit(twd),
        .trst_limit(trst),
        .arm_delay_limit(arm_dly)
    );

    // Output mapping
    // Since WDO is open-drain emulation (1'bz or 1'b0),
    // we need to invert it for the LED (LED is active high)
    assign wdo_led = (wdo_internal === 1'bz) ? 1'b0 : 1'b1; // LED ON when fault
    assign wdo_pin = wdo_internal;
    assign enout_led = enout_internal; // LED D3 shows if WD is enabled

endmodule
