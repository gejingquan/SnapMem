`timescale 1ns / 1ps
//////////////////////////////////////////////////////////////////////////////////
// Company: 
// Engineer: 
// 
// Create Date: 12/03/2020 03:39:42 PM
// Design Name: 
// Module Name: pump
// Project Name: 
// Target Devices: 
// Tool Versions: 
// Description: 
// 
// Dependencies: 
// 
// Revision:
// Revision 0.01 - File Created
// Additional Comments:
// 
//////////////////////////////////////////////////////////////////////////////////


module pump(
	input wire clk,
	input wire rstn,
	(*mark_debug = "true"*)output wire wr_en,
	(*mark_debug = "true"*)output wire [31:0] wr_addr,
	(*mark_debug = "true"*)output wire [31:0] wr_data,
	(*mark_debug = "true"*)input wire wr_done,
    
	(*mark_debug = "true"*)output wire rd_en,
	(*mark_debug = "true"*)output wire [31:0] rd_addr,
	(*mark_debug = "true"*)input wire rd_valid,
	(*mark_debug = "true"*)input wire [31:0] rd_data,
	(*mark_debug = "true"*)input wire rd_done,
     
	(*mark_debug = "true"*)input wire [31:0] FlashMem_id,
	(*mark_debug = "true"*)input wire [31:0] pump_addr,
	(*mark_debug = "true"*)input wire [31:0] pump_size,
	(*mark_debug = "true"*)input wire [31:0] pump_controller
	);

	(*mark_debug = "true"*)reg [31:0]  read_base_addr;
	(*mark_debug = "true"*)reg [31:0]  read_max_addr;
	(*mark_debug = "true"*)reg [31:0]  write_base_addr; 
	(*mark_debug = "true"*)reg [31:0]  write_max_addr;


	
	
	(*mark_debug = "true"*)reg [31:0] rd_addr_reg;
	(*mark_debug = "true"*)reg [31:0] wr_addr_reg;
	(*mark_debug = "true"*)reg [31:0] wr_addr_next_reg;
	(*mark_debug = "true"*)reg rd_en_reg;
	(*mark_debug = "true"*)reg wr_en_reg;
	(*mark_debug = "true"*)reg [31:0] wr_data_reg;
	(*mark_debug = "true"*)reg [31:0] wr_data_next_reg;
	
	
	assign rd_addr = rd_addr_reg;
	assign wr_addr = wr_addr_reg;
	assign rd_en   = rd_en_reg;
	assign wr_en   = wr_en_reg;
	assign wr_data = wr_data_reg;


	//state machine
	(*mark_debug = "true"*)reg [12:0] state;
	parameter [12:0]  IDLE        = 13'b0_0000_0000_0001;
	parameter [12:0]  WR          = 13'b0_0000_0000_0010;
	parameter [12:0]  WR_DONE     = 13'b0_0000_0000_0100;
	parameter [12:0]  RD          = 13'b0_0000_0000_1000;
	parameter [12:0]  RD_DONE     = 13'b0_0000_0001_0000;
          
	parameter [12:0]  FLUSH       = 13'b0_0000_0010_0000;
	parameter [12:0]  FLUSH_DONE  = 13'b0_0000_0100_0000;    
	
	parameter [12:0]  CONTROLLER_CLEAR       = 13'b0_0001_0000_0000;
	parameter [12:0]  CONTROLLER_CLEAR_DONE  = 13'b0_0010_0000_0000; 
	
	//FlashMem base address
	parameter [31:0] FLASHMEM_BASE_ADDR = 32'hA000_0000;
	parameter [31:0] PUMP_BASE_ADDR     = 32'hA001_0000;
	
	//pump_controller        
	parameter [31:0] PUMP_IN   = 32'hF0F0F0F0;
	parameter [31:0] PUMP_OUT  = 32'h0F0F0F0F;
	


	
	//====================================================
	//state machine and base registers
	//====================================================
	always @(posedge clk) 
		begin
		if (rstn == 1'b0) 
			begin
			state <= IDLE;
			end	
		else
			begin
			case(state)
				IDLE:
					begin
					if(pump_controller == PUMP_IN || pump_controller == PUMP_OUT) 
						begin
						state <= CONTROLLER_CLEAR;
						end
					else 
						begin
                        state <= IDLE;
						end
					
					end
				CONTROLLER_CLEAR:
					begin
					state <= CONTROLLER_CLEAR_DONE;
					end
				CONTROLLER_CLEAR_DONE:
					begin
					if (wr_done == 1'b1) 
						begin
						state <= RD;
						end
					else 
						begin
						state <= state;
						end
					end								
				RD: 
					begin
					state <= RD_DONE;
					end 
				RD_DONE:
					begin
					if (rd_done == 1'b1) 
						begin
						state <= FLUSH;
						end    
					else 
						begin
						state <= state;
						end
					end				
				FLUSH:
					begin
					state <= FLUSH_DONE;
					end 				
				FLUSH_DONE:
					begin
					if (wr_done == 1'b1) 
						begin
						state <= WR;
						end
					else 
						begin
						state <= state;
						end
					end				
				WR:
					begin
					state <= WR_DONE;
					end				
				WR_DONE:
					begin
					if ((wr_done == 1'b1) && (wr_addr_reg != write_max_addr)) 
						begin
						state <= RD;
						end
					else if ((wr_done == 1'b1) && (wr_addr_reg == write_max_addr)) 
						begin
						state <= IDLE;
						end 
					else 
						begin
						state <= state;
						end
					end 				
				default: 
					begin
					state = IDLE;
					end					
			endcase
			end
		end

	//====================================================
	//read and write address and data
	//====================================================  
	always @(posedge clk) 	
		begin
		if (rstn == 1'b0) 
			begin
			rd_en_reg <= 1'b0;
			wr_en_reg <= 1'b0;
			rd_addr_reg <= 32'h0;
			wr_addr_reg <= 32'h0;
			wr_data_reg <= 32'h0;
			wr_addr_next_reg <= 32'h0;
			wr_data_next_reg <= 32'h0;
			end			
		else 
			begin
            case (state)
				IDLE:
					begin
					rd_en_reg    <= 1'b0;
					wr_en_reg    <= 1'b0;					
					wr_data_reg  <= 32'h0;
					wr_data_next_reg <= 32'h0;
					if(pump_controller == PUMP_IN) 
						begin
						read_base_addr       <= pump_addr;
						rd_addr_reg          <= pump_addr;
						wr_addr_reg          <= PUMP_BASE_ADDR + 32'd12;						
						read_max_addr        <= pump_addr+pump_size;                      
						write_base_addr      <= FLASHMEM_BASE_ADDR + FlashMem_id*32'h2000;
						wr_addr_next_reg     <= FLASHMEM_BASE_ADDR + FlashMem_id*32'h2000;
						write_max_addr       <= FLASHMEM_BASE_ADDR + FlashMem_id*32'h2000 + pump_size;                                                                      						
						end
					else if(pump_controller == PUMP_OUT)
						begin
                        read_base_addr       <= FLASHMEM_BASE_ADDR + FlashMem_id*32'h2000;
						rd_addr_reg          <= FLASHMEM_BASE_ADDR + FlashMem_id*32'h2000;
						wr_addr_reg          <= PUMP_BASE_ADDR + 32'd12;
                        read_max_addr        <= FLASHMEM_BASE_ADDR + FlashMem_id*32'h2000 + pump_size;                       
                        write_base_addr      <= pump_addr;
						wr_addr_next_reg     <= pump_addr;
                        write_max_addr       <= pump_addr+pump_size;					
						end
					else
						begin
						read_base_addr       <= 32'h0;
						rd_addr_reg          <= 32'h0;
						wr_addr_reg          <= 32'h0;
						read_max_addr        <= 32'h0;                        
						write_base_addr      <= 32'h0;
						wr_addr_next_reg     <= 32'h0;
						write_max_addr       <= 32'h0;
						end
					end
				CONTROLLER_CLEAR:
					begin
					wr_en_reg   <= 1'b1;
					end				
				CONTROLLER_CLEAR_DONE:
					begin
					wr_en_reg   <= 1'b0;
					end					
				RD:
					begin
					rd_en_reg <= 1'b1;
					rd_addr_reg  <= rd_addr_reg;					
					end
				RD_DONE:
					begin
					rd_en_reg <= 1'b0;
					if (rd_done == 1'b1) 
						begin
						wr_data_next_reg <= rd_data;
						wr_data_reg <= 32'h0;
						wr_addr_reg <= rd_addr_reg;						
						end
					else 
						begin
						wr_data_next_reg <= wr_data_next_reg;
						end					
					end
				FLUSH:
					begin
					wr_en_reg   <= 1'b1;
					end				
				FLUSH_DONE:
					begin
					wr_en_reg   <= 1'b0;
					if(wr_done == 1'b1)
						begin
						wr_data_reg <= wr_data_next_reg;
						wr_addr_reg <= wr_addr_next_reg;						
						if(rd_addr_reg == read_max_addr)
							begin
							rd_addr_reg <= read_base_addr;
							end
						else
							begin
							rd_addr_reg <= rd_addr_reg + 32'h4;
							end
						end
					else
						begin
						rd_addr_reg <= rd_addr_reg;
						end
					end				
				WR:
					begin
					wr_en_reg   <= 1'b1;
					end				
				WR_DONE:
					begin
					wr_en_reg   <= 1'b0;
					if(wr_done == 1'b1)
						begin
						if(wr_addr_reg == write_max_addr)
							begin
							wr_addr_next_reg <= write_base_addr;
							end
						else
							begin
							wr_addr_next_reg <= wr_addr_next_reg + 32'h4;
							end
						end
					else
						begin
						wr_addr_reg <= wr_addr_reg;
						end
					end				
				default:
					begin
					rd_en_reg <= 1'b0;
					wr_en_reg <= 1'b0;
					rd_addr_reg <= 32'h0;
					wr_addr_reg <= 32'h0;
					wr_data_reg <= 32'h0;
					wr_addr_next_reg <= 32'h0;
					wr_data_next_reg <= 32'h0;					
					end
				
			endcase		
			end
		end
			
endmodule