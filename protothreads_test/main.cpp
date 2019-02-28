#include <vla/registers.hpp>
#include <vla/serial.hpp>
#include <util/delay.h>
#include <stdint.h>

#include <vla/protothreads.hpp>
using namespace vla;

using message_t = uint8_t;
using queue_t = msg_queue<message_t, 5>;

static auto &ser = get_serial_async_debug();

static const char hex_chr[] = {'0', '1', '2', '3',
                               '4', '5', '6', '7',
                               '8', '9', 'a', 'b',
                               'c', 'd', 'e', 'f'};
static const char* hex(uint8_t n) {
    static char hex_buffer[] = "0x00";
    hex_buffer[2] = hex_chr[n>>4];
    hex_buffer[3] = hex_chr[n & 0xf];
    return hex_buffer;
}

message_t message_read()
{
    // read serial but do not block
    message_t m;
    if (read(ser, &m, 1)) {
        return m;
    }
    return 0;
}

void write_message(message_t m)
{
    write_line(ser, hex(m));
}

/**
 * Call message_read to get messages. If there is no message available
 * or the buffer is full yield. If the message is ready and the queue
 * is not full, put the message in the queue.
 *
 * Both producer and consumer are greedy, meaning they run until they
 * have to block. In the case of the consumer this means it runs until
 * a) there is no input to read or b) the output queue q is full. In
 * the case of the producer this means that ir runs until its input
 * queue is empty. Uncommenting the pt_yield calls has both the
 * producer and the consume relinquish the CPU after just one message
 * send or received. This has the effect of intertwining the
 * executions of each thread. Which is better depends on the
 * application.
 */
void producer(pt *pt, queue_t &q, message_t &msg) {
	pt_begin(pt);
	for (;;) {
        pt_wait(pt, msg = message_read());
		pt_wait(pt, !q.full());
        q.push(msg);
        // pt_yield(pt); // see comment on producer
	}
	pt_end(pt);
}

/**
 * Read messages from the queue. If there are no pending messages
 * yield.
 */
void consumer(pt *pt, queue_t &q) {
	pt_begin(pt);
	for (;;) {
		pt_wait(pt, !q.empty());
		auto m = q.pop();
        write_message(m);
        // pt_yield(pt); // see comment on producer
	}
	pt_end(pt);
}

int main(int argc, char **argv)
{
    write_line(ser, "BEGIN");
    pt pt_producer = pt_init();
    pt pt_consumer = pt_init();
    auto queue = queue_t{};
    message_t msg;
    while (true) {
        write_line(ser, "GO");
        for (int i = 0; i < 1024; ++i) {
            producer(&pt_producer, queue, msg);
            consumer(&pt_consumer, queue);
        }
        write_line(ser, "WAIT 5s");
        _delay_ms(5000);
    }
}
