`timescale 1 ns / 1 ps

	module master_axi_lite_v1_0_M00_AXI #
	(
		// Users to add parameters here

		// User parameters ends
		// Do not modify the parameters beyond this line

		// The master will start generating data from the C_M_START_DATA_VALUE value
		parameter  C_M_START_DATA_VALUE	= 32'hAA000000,
		// The master requires a target slave base address.
    // The master will initiate read and write transactions on the slave with base address specified here as a parameter.
		parameter  C_M_TARGET_SLAVE_BASE_ADDR	= 32'h40000000,
		// Width of M_AXI address bus. 
    // The master generates the read and write addresses of width specified as C_M_AXI_ADDR_WIDTH.
		parameter integer C_M_AXI_ADDR_WIDTH	= 32,
		// Width of M_AXI data bus. 
    // The master issues write data and accept read data where the width of the data bus is C_M_AXI_DATA_WIDTH
		parameter integer C_M_AXI_DATA_WIDTH	= 32,
		// Transaction number is the number of write 
    // and read transactions the master will perform as a part of this example memory test.
		parameter integer C_M_TRANSACTIONS_NUM	= 4
	)
	
    (
		// Users to add ports here
        //写接口
        input wire wr_en,//写使能
        input wire [31:0] wr_addr,//相对写地址，与写使能对齐
        input wire [31:0] wr_data,//写数据，与写使能对齐
        output wire wr_done,//写结束，单个时钟周期有效
        //读接口
        input wire rd_en,//读使能，单个时钟周期有效
        input wire [31:0] rd_addr,//相对读地址，与读使能对齐
        output wire rd_valid,//读出数据有效，单个时钟周期有效
        output wire [31:0] rd_data,//读出数据
        output wire rd_done,//读结束，单个时钟周期有效

		// User ports ends
		// Do not modify the ports beyond this line	
		

        //  // Initiate AXI transactions
		// input wire  INIT_AXI_TXN,
		// // Asserts when ERROR is detected
		// output reg  ERROR,
		// // Asserts when AXI transactions is complete
		// output wire  TXN_DONE,
		// AXI clock signal
		input wire  M_AXI_ACLK,
		// AXI active low reset signal
		input wire  M_AXI_ARESETN,
		// Master Interface Write Address Channel ports. Write address (issued by master)
		output wire [C_M_AXI_ADDR_WIDTH-1 : 0] M_AXI_AWADDR,
		// Write channel Protection type.
        // This signal indicates the privilege and security level of the transaction,
        // and whether the transaction is a data access or an instruction access.
		output wire [2 : 0] M_AXI_AWPROT,
		// Write address valid. 
        // This signal indicates that the master signaling valid write address and control information.
		output wire  M_AXI_AWVALID,
		// Write address ready. 
        // This signal indicates that the slave is ready to accept an address and associated control signals.
		input wire  M_AXI_AWREADY,
		// Master Interface Write Data Channel ports. Write data (issued by master)
		output wire [C_M_AXI_DATA_WIDTH-1 : 0] M_AXI_WDATA,
		// Write strobes. 
        // This signal indicates which byte lanes hold valid data.
        // There is one write strobe bit for each eight bits of the write data bus.
		output wire [C_M_AXI_DATA_WIDTH/8-1 : 0] M_AXI_WSTRB,
		// Write valid. This signal indicates that valid write data and strobes are available.
		output wire  M_AXI_WVALID,
		// Write ready. This signal indicates that the slave can accept the write data.
		input wire  M_AXI_WREADY,
		// Master Interface Write Response Channel ports. 
		
		
        // This signal indicates the status of the write transaction.
  	    input wire [1 : 0] M_AXI_BRESP,
		// Write response valid. 
        // This signal indicates that the channel is signaling a valid write response
		input wire  M_AXI_BVALID,
		// Response ready. This signal indicates that the master can accept a write response.
		output wire  M_AXI_BREADY,
		// Master Interface Read Address Channel ports. Read address (issued by master)
		output wire [C_M_AXI_ADDR_WIDTH-1 : 0] M_AXI_ARADDR,
		// Protection type. 
        // This signal indicates the privilege and security level of the transaction, 
        // and whether the transaction is a data access or an instruction access.
		output wire [2 : 0] M_AXI_ARPROT,
		// Read address valid. 
        // This signal indicates that the channel is signaling valid read address and control information.
		output wire  M_AXI_ARVALID,
		// Read address ready. 
        // This signal indicates that the slave is ready to accept an address and associated control signals.
		input wire  M_AXI_ARREADY,
		// Master Interface Read Data Channel ports. Read data (issued by slave)
		input wire [C_M_AXI_DATA_WIDTH-1 : 0] M_AXI_RDATA,
		// Read response. This signal indicates the status of the read transfer.
		input wire [1 : 0] M_AXI_RRESP,
		// Read valid. This signal indicates that the channel is signaling the required read data.
		input wire  M_AXI_RVALID,
		// Read ready. This signal indicates that the master can accept the read data and response information.
		output wire  M_AXI_RREADY
	);  



	assign M_AXI_AWPROT	= 3'b000;
	assign M_AXI_WSTRB	= 4'b1111;
	assign M_AXI_ARPROT	= 3'b001;
    
    reg [31:0] M_AXI_AWADDR_REG = 32'd0; assign M_AXI_AWADDR = M_AXI_AWADDR_REG;
    reg M_AXI_AWVALID_REG = 1'b0; assign M_AXI_AWVALID = M_AXI_AWVALID_REG;
    reg [31:0] M_AXI_WDATA_REG = 32'd0; assign M_AXI_WDATA = M_AXI_WDATA_REG;
    reg M_AXI_WVALID_REG = 1'b0; assign M_AXI_WVALID = M_AXI_WVALID_REG;
    reg M_AXI_BREADY_REG = 1'b0; assign M_AXI_BREADY = M_AXI_BREADY_REG;
    
    reg [31:0] M_AXI_ARADDR_REG = 32'd0; assign M_AXI_ARADDR = M_AXI_ARADDR_REG;
    reg M_AXI_ARVALID_REG = 1'b0; assign M_AXI_ARVALID = M_AXI_ARVALID_REG;
    reg M_AXI_RREADY_REG = 1'b0; assign M_AXI_RREADY = M_AXI_RREADY_REG;


	// Add user logic here
    
	//写操作*******************************************************************
	always @(posedge M_AXI_ACLK) begin
        if (wr_en == 1'b1) begin
			//地址和数据同时有效
			M_AXI_WDATA_REG <= wr_data;
			M_AXI_WVALID_REG <= 1'b1;
        end
        else if ((M_AXI_WVALID_REG == 1'b1) && (M_AXI_WREADY == 1'b1)) begin
			//收到slave响应
			M_AXI_WDATA_REG <= M_AXI_WDATA_REG;
			M_AXI_WVALID_REG <= 1'b0;
        end
		else begin
			//保持
			M_AXI_WDATA_REG <= M_AXI_WDATA_REG;
			M_AXI_WVALID_REG <= M_AXI_WVALID_REG;
		end
    end    
    

	always @(posedge M_AXI_ACLK) begin
		if (wr_en == 1'b1) begin
			//地址和数据同时有效
			M_AXI_AWADDR_REG <= wr_addr;
			M_AXI_AWVALID_REG <= 1'b1;
		end
		else if ((M_AXI_AWVALID_REG == 1'b1) && (M_AXI_AWREADY == 1'b1)) begin
			//收到slave响应
			M_AXI_AWADDR_REG <= M_AXI_AWADDR_REG;
			M_AXI_AWVALID_REG <= 1'b0;
		end
		else begin
			//收到slave响应
			M_AXI_AWADDR_REG <= M_AXI_AWADDR_REG;
			M_AXI_AWVALID_REG <= M_AXI_AWVALID_REG;
		end
	end        
    
 
 	always @(posedge M_AXI_ACLK) begin
		case (M_AXI_BREADY_REG)
			1'b0: begin
				//收到bvalid即设置为1
				if (M_AXI_BVALID == 1'b1) begin
					M_AXI_BREADY_REG <= 1'b1;
				end
				else begin
					M_AXI_BREADY_REG <= M_AXI_BREADY_REG;
				end
			end
			
			1'b1: begin
				//高电平仅1个时钟周期
				M_AXI_BREADY_REG <= 1'b0;
			end
		endcase
	end
	
	assign wr_done = M_AXI_BREADY_REG;
	
	
	//读操作************************************************
	always @(posedge M_AXI_ACLK) begin
		if (rd_en == 1'b1) begin
			M_AXI_ARADDR_REG <= rd_addr;
			M_AXI_ARVALID_REG <= 1'b1;
		end
		else if ((M_AXI_ARVALID_REG == 1'b1) && (M_AXI_ARREADY == 1'b1)) begin
			M_AXI_ARADDR_REG <= M_AXI_ARADDR_REG;
			M_AXI_ARVALID_REG <= 1'b0;
		end
		else begin
			//保持
			M_AXI_ARADDR_REG <= M_AXI_ARADDR_REG;
			M_AXI_ARVALID_REG <= M_AXI_ARVALID_REG;
		end
	end
	
	
	always @(posedge M_AXI_ACLK) begin
		case (M_AXI_RREADY_REG)
			1'b0: begin
				if (M_AXI_RVALID == 1'b1) begin
					M_AXI_RREADY_REG <= 1'b1;
				end
				else begin
					M_AXI_RREADY_REG <= M_AXI_RREADY_REG;
				end
			end
			
			1'b1: begin
				M_AXI_RREADY_REG <= 1'b0;
			end
		endcase
	end
	
	assign rd_done = M_AXI_RREADY_REG;
	assign rd_valid = M_AXI_RREADY_REG;
	
	reg [31:0] rd_data_reg = 32'd0;
	assign rd_data = rd_data_reg;
	
	always @(posedge M_AXI_ACLK) begin
		if ((M_AXI_RVALID == 1'b1) && (M_AXI_RREADY_REG == 1'b0)) begin
			rd_data_reg <= M_AXI_RDATA;
		end
		else begin
			rd_data_reg <= rd_data_reg;
		end
	end

	// User logic ends

	endmodule	
	
	
		
	
    
    
    		