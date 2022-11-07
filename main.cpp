#include "mbed.h"

Thread thread_master;
Thread thread_slave;

// master
SPI spi(D11, D12, D13);  // mosi, miso, sclk
DigitalOut cs(D9);

// slave
SPISlave device(PD_4, PD_3, PD_1, PD_0);  // mosi, miso, sclk, cs; PMOD pins

DigitalOut led(LED3);

int mode = 0x01;

int slave() {
    device.format(8, 3);
    device.frequency(1000000);
    // device.reply(0x00); // Prime SPI with first reply
    while (1) {
        if (device.receive()) {
            int mode = device.read();  // Read mode from master
            printf("mode = %0x\n", mode);
            if (mode == 0x01) {  // mode 01
                int v;
                v = 0x01;
                device.reply(v);    // Make this the next reply
                v = device.read();  // Confirmation to master
                v = device.read();  // Read another byte from master
                printf("Read from master: v = %d\n", v);
                v = v + 1;
                device.reply(v);        // Make this the next reply
                v = device.read();      // Read again to allow master read back
            } else if (mode == 0x02) {  // mode 02
                int v;
                v = 0x02;
                device.reply(v);    // Make this the next reply
                v = device.read();  // Confirmation to master
                v = device.read();  // Read another byte from master
                printf("Read from master: v = %d\n", v);
                v = v + 2;
                device.reply(v);    // Make this the next reply
                v = device.read();  // Read again to allow master read back
            } else {                // Undefined mode
                printf("Default reply to master: 0x00\n");
                device.reply(0x00);  // Reply default value
                int v;
                v = 0x00;
                device.reply(v);    // Make this the next reply
                v = device.read();  // Confirmation to master
                v = device.read();  // Read another byte from master
                printf("Read from master: v = %d\n", v);
                v = v + 0;
                device.reply(v);    // Make this the next reply
                v = device.read();  // Read again to allow master read back
            };
        }
    }
}

void master() {
    int number = 0;

    // Setup the spi for 8 bit data, high steady state clock,
    // second edge capture, with a 1MHz clock rate
    spi.format(8, 3);
    spi.frequency(1000000);

    for (int i = 0; i < 5; ++i, mode++) {  // Run for 5 times
        // Chip must be deselected
        cs = 1;
        // Select the device by seting chip select low
        cs = 0;

        printf("New chapter starts from here.\n");

        int response = spi.write(mode);  // Send type of mode
        cs = 1;                          // Deselect the device
        ThisThread::sleep_for(100ms);    // Wait for debug print
        printf("First response from slave = %d\n", response);

        // Select the device by seting chip select low
        cs = 0;

        response = spi.write(mode);    // Read slave reply
        ThisThread::sleep_for(100ms);  // Wait for debug print
        printf("The mode received from slave = %d\n", response);
        response = spi.write(0x88);    // extra data to send due to requirement
        ThisThread::sleep_for(100ms);  // Wait for debug print
        response = spi.write(mode);    // Read the answer from slave
        ThisThread::sleep_for(100ms);  // Wait for debug print
        printf("Answer from slave = %d\n", response);
        cs = 1;  // Deselect the device
    }
}

int main() {
    thread_slave.start(slave);
    thread_master.start(master);
}