#include <vla/registers.hpp>
#include <vla/serial.hpp>
#include <util/delay.h>
#include <stdint.h>

#include <vla/protothreads.hpp>
using namespace vla;

/**
 * Read chars from the serial port and in one thread and output them
 * on hex format also in the serial port. Implemented using two
 * protothreads, one for reading and one for writing which communicate
 * through a thread safe message queue.
 */

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
class producer_t: public ptxx_thread {
    queue_t &q;
    // msg is a member variable so that it is possible to store the
    // state of the thread from invocation to invocation
    message_t msg{};
public:
    producer_t(queue_t &q):q(q){}
    void operator()() {
        ptxx_begin();
        for (;;) {
            ptxx_wait(msg = message_read());
            ptxx_wait(!q.full());
            q.push(msg);
            pt_yield(&ctx); // see comment on producer
        }
        ptxx_end();
    }
};

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
    auto queue = queue_t{};
    auto producer = producer_t{queue};
    pt pt_consumer = pt_init();
    while (true) {
        write_line(ser, "GO");
        for (int i = 0; i < 1024; ++i) {
            producer();
            consumer(&pt_consumer, queue);
        }
        write_line(ser, "WAIT 5s");
        _delay_ms(5000);
    }
}
