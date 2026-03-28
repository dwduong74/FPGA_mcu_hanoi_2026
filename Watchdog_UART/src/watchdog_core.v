module watchdog_core (
    input clk,          // 27 MHz
    input rst_n,        // Reset active low
    
    input en,           // Enable watchdog
    input wdi,          // Kick signal (Watchdog Input)
    
    output wdo,         // Watchdog Output (Active Low)
    output enout,       // Enable output (Reflects internal enable state)
    
    // Config parameters from UART (in clock cycles)
    input [31:0] twd_limit,
    input [31:0] trst_limit,
    input [31:0] arm_delay_limit
);

    // Internal states
    localparam IDLE    = 3'd0;
    localparam ARMING  = 3'd1;
    localparam MONITOR = 3'd2;
    localparam FAULT   = 3'd3;
    
    reg [2:0] state;
    reg [31:0] counter;
    reg wdo_internal;
    
    // WDI edge detection
    reg wdi_reg1, wdi_reg2;
    wire wdi_edge = wdi_reg1 ^ wdi_reg2;
    
    always @(posedge clk or negedge rst_n) begin
        if (!rst_n) begin
            wdi_reg1 <= 1'b0;
            wdi_reg2 <= 1'b0;
        end else begin
            wdi_reg1 <= wdi;
            wdi_reg2 <= wdi_reg1;
        end
    end

    // Watchdog State Machine
    always @(posedge clk or negedge rst_n) begin
        if (!rst_n) begin
            state <= IDLE;
            counter <= 32'd0;
            wdo_internal <= 1'b1; // Normal high
        end else if (!en) begin
            state <= IDLE;
            counter <= 32'd0;
            wdo_internal <= 1'b1;
        end else begin
            case (state)
                IDLE: begin
                    if (en) begin
                        state <= ARMING;
                        counter <= 32'd0;
                    end
                end
                
                ARMING: begin
                    if (counter >= arm_delay_limit) begin
                        state <= MONITOR;
                        counter <= 32'd0;
                    end else begin
                        counter <= counter + 1'b1;
                    end
                end
                
                MONITOR: begin
                    if (wdi_edge) begin
                        counter <= 32'd0;
                    end else if (counter >= twd_limit) begin
                        state <= FAULT;
                        counter <= 32'd0;
                        wdo_internal <= 1'b0; // Fault detected
                    end else begin
                        counter <= counter + 1'b1;
                    end
                end
                
                FAULT: begin
                    if (counter >= trst_limit) begin
                        state <= MONITOR; // Auto-recovery (or IDLE depending on design)
                        counter <= 32'd0;
                        wdo_internal <= 1'b1;
                    end else begin
                        counter <= counter + 1'b1;
                        wdo_internal <= 1'b0;
                    end
                end
                
                default: state <= IDLE;
            endcase
        end
    end
    
    // Open-drain emulation for WDO
    assign wdo = (wdo_internal) ? 1'bz : 1'b0;
    assign enout = en;

endmodule
