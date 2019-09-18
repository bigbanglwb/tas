#include <assert.h>
#include <stdio.h>
#include <stdlib.h>

#include "../testutils.h"

#include <rte_config.h>
#include <rte_ether.h>
#include <rte_mbuf.h>

#include <tas.h>
#include <tas_memif.h>
#include "../../tas/fast/internal.h"
#include "../../tas/fast/fastemu.h"

#define TEST_IP   0x0a010203
#define TEST_PORT 12345

#define TEST_LIP   0x0a010201
#define TEST_LPORT 23456


#if RTE_VER_YEAR < 19
  typedef struct ether_addr macaddr_t;
#else
  typedef struct rte_ether_addr macaddr_t;
#endif
macaddr_t eth_addr;

void *tas_shm = (void *) 0;

struct flextcp_pl_mem state_base;
struct flextcp_pl_mem *fp_state = &state_base;

struct dataplane_context **ctxs = NULL;

struct qman_set_op {
  int got_op;
  uint32_t id;
  uint32_t rate;
  uint32_t avail;
  uint16_t max_chunk;
  uint8_t flags;
} qm_set_op;

int qman_set(struct qman_thread *t, uint32_t id, uint32_t rate, uint32_t avail,
    uint16_t max_chunk, uint8_t flags)
{
  qm_set_op.got_op = 1;
  qm_set_op.id = id;
  qm_set_op.rate = rate;
  qm_set_op.avail = avail;
  qm_set_op.max_chunk = max_chunk;
  qm_set_op.flags = flags;

  return 0;
}

void util_flexnic_kick(struct flextcp_pl_appctx *ctx, uint32_t ts_us)
{
  printf("util_flexnic_kick\n");
}

/* initialize basic flow state */
static void flow_init(uint32_t fid, uint32_t rxlen, uint32_t txlen, uint64_t opaque)
{
  struct flextcp_pl_flowst *fs = &state_base.flowst[fid];
  void *rxbuf = test_zalloc(rxlen);
  void *txbuf = test_zalloc(txlen);

  fs->opaque = opaque;
  fs->rx_base_sp = (uintptr_t) rxbuf;
  fs->tx_base = (uintptr_t) txbuf;
  fs->rx_len = rxlen;
  fs->tx_len = txlen;
  fs->local_ip = t_beui32(TEST_LIP);
  fs->remote_ip = t_beui32(TEST_IP);
  fs->local_port = t_beui16(TEST_LPORT);
  fs->remote_port = t_beui16(TEST_PORT);
  fs->rx_avail = rxlen;
  fs->rx_remote_avail = rxlen;
  fs->tx_rate = 10000;
  fs->rtt_est = 18;
}

/* alloc dummy mbuf */
static struct rte_mbuf *mbuf_alloc(void)
{
  struct rte_mbuf *tmb = calloc(1, 2048);
  tmb->data_off = 256;
  tmb->buf_addr = (uint8_t *) (tmb + 1) + tmb->data_off;
  tmb->buf_len = 2048 - sizeof(*tmb);
  return tmb;
}

void test_txbump_small(void *arg)
{
  int ret;
  struct flextcp_pl_flowst *fs = &state_base.flowst[0];
  struct dataplane_context ctx;
  memset(&ctx, 0, sizeof(ctx));

  flow_init(0, 1024, 1024, 123456);

  struct rte_mbuf *tmb = mbuf_alloc();

  ret = fast_flows_bump(&ctx, 0, 0, 0, 32, 0, (struct network_buf_handle *) tmb, 0);
  test_assert("unused tx buffer", ret == -1);
  test_assert("updated tx avail", fs->tx_avail == 32);
  test_assert("qman set sent", qm_set_op.got_op);
  test_assert("qman set id correct", qm_set_op.id == 0);
  test_assert("qman set rate correct", qm_set_op.rate == fs->tx_rate);
  test_assert("qman set avail correct", qm_set_op.avail == 32);
  test_assert("qman set max chunk correct", qm_set_op.max_chunk == 1448);
  test_assert("qman set flags", qm_set_op.flags ==
      (QMAN_SET_RATE | QMAN_SET_MAXCHUNK | QMAN_ADD_AVAIL));
}

void test_txbump_full(void *arg)
{
  int ret;
  struct flextcp_pl_flowst *fs = &state_base.flowst[0];
  struct dataplane_context ctx;
  memset(&ctx, 0, sizeof(ctx));

  flow_init(0, 1024, 1024, 123456);

  struct rte_mbuf *tmb = mbuf_alloc();

  ret = fast_flows_bump(&ctx, 0, 0, 0, 1024, 0, (struct network_buf_handle *) tmb, 0);
  test_assert("unused tx buffer", ret == -1);
  test_assert("updated tx avail", fs->tx_avail == 1024);
  test_assert("qman set sent", qm_set_op.got_op);
  test_assert("qman set id correct", qm_set_op.id == 0);
  test_assert("qman set rate correct", qm_set_op.rate == fs->tx_rate);
  test_assert("qman set avail correct", qm_set_op.avail == 1024);
  test_assert("qman set max chunk correct", qm_set_op.max_chunk == 1448);
  test_assert("qman set flags", qm_set_op.flags ==
      (QMAN_SET_RATE | QMAN_SET_MAXCHUNK | QMAN_ADD_AVAIL));
}

void test_txbump_toolong(void *arg)
{
  int ret;
  struct flextcp_pl_flowst *fs = &state_base.flowst[0];
  struct dataplane_context ctx;
  memset(&ctx, 0, sizeof(ctx));

  flow_init(0, 1024, 1024, 123456);

  struct rte_mbuf *tmb = mbuf_alloc();

  ret = fast_flows_bump(&ctx, 0, 0, 0, 2048, 0, (struct network_buf_handle *) tmb, 0);
  test_assert("unused tx buffer", ret == -1);
  test_assert("updated tx avail", fs->tx_avail == 0);
  test_assert("qman set sent", !qm_set_op.got_op);
}

void test_rxbump_toolong(void *arg)
{
  int ret;
  struct flextcp_pl_flowst *fs = &state_base.flowst[0];
  struct dataplane_context ctx;
  memset(&ctx, 0, sizeof(ctx));

  flow_init(0, 1024, 1024, 123456);
  fs->rx_avail = 0;

  struct rte_mbuf *tmb = mbuf_alloc();

  ret = fast_flows_bump(&ctx, 0, 0, 2048, 0, 0, (struct network_buf_handle *) tmb, 0);
  test_assert("unused tx buffer", ret == -1);
  test_assert("updated tx avail", fs->rx_avail == 0);
  test_assert("qman set sent", !qm_set_op.got_op);
}


/* Test rx bump where the flow control window opens up from zero with no tx data
 * available. In this case the fast path should issue an ack to open up the flow
 * control window.
 */
void test_rxbump_fc_reopen_notx(void *arg)
{
  int ret;
  struct flextcp_pl_flowst *fs = &state_base.flowst[0];
  struct dataplane_context ctx;
  memset(&ctx, 0, sizeof(ctx));

  flow_init(0, 1024, 1024, 123456);
  fs->rx_avail = 0;

  struct rte_mbuf *tmb = mbuf_alloc();

  ret = fast_flows_bump(&ctx, 0, 0, 1024, 0, 0, (struct network_buf_handle *) tmb, 0);
  test_assert("used tx buffer", ret == 0);
  test_assert("tx queue num done", ctx.tx_num == 1);
  test_assert("tx buf in tx queue",
      ctx.tx_handles[0] == (struct network_buf_handle *) tmb);
  test_assert("rx avail updated", fs->rx_avail == 1024);
  test_assert("qman set sent", !qm_set_op.got_op);
  /* TODO: check ack packet */
}

/* Test rx bump where the flow control window opens up from zero with tx data
 * available and an open tx flow control window. In this case the fast path
 * does not have to issue an ack.
 */
void test_rxbump_fc_reopen_tx(void *arg)
{
  int ret;
  struct flextcp_pl_flowst *fs = &state_base.flowst[0];
  struct dataplane_context ctx;
  memset(&ctx, 0, sizeof(ctx));

  flow_init(0, 1024, 1024, 123456);
  fs->rx_avail = 0;
  fs->tx_avail = 32;

  struct rte_mbuf *tmb = mbuf_alloc();

  ret = fast_flows_bump(&ctx, 0, 0, 1024, 0, 0, (struct network_buf_handle *) tmb, 0);
  test_assert("used tx buffer", ret == -1);
  test_assert("tx queue num done", ctx.tx_num == 0);
  test_assert("rx avail updated", fs->rx_avail == 1024);
  test_assert("qman set sent", !qm_set_op.got_op);
}

/* Test rx bump where the flow control window opens up from zero with tx data
 * available and a closed tx flow control window. In this case the fast path
 * has to generate an ack.
 */
void test_rxbump_fc_reopen_deadlock(void *arg)
{
  int ret;
  struct flextcp_pl_flowst *fs = &state_base.flowst[0];
  struct dataplane_context ctx;
  memset(&ctx, 0, sizeof(ctx));

  flow_init(0, 1024, 1024, 123456);
  fs->rx_avail = 0;
  fs->rx_remote_avail = 0;
  fs->tx_avail = 32;

  struct rte_mbuf *tmb = mbuf_alloc();

  ret = fast_flows_bump(&ctx, 0, 0, 1024, 0, 0, (struct network_buf_handle *) tmb, 0);
  test_assert("used tx buffer", ret == 0);
  test_assert("tx queue num done", ctx.tx_num == 1);
  test_assert("tx buf in tx queue",
      ctx.tx_handles[0] == (struct network_buf_handle *) tmb);
  test_assert("rx avail updated", fs->rx_avail == 1024);
  test_assert("qman set sent", !qm_set_op.got_op);
  /* TODO: check ack packet */
}


int main(int argc, char *argv[])
{
  int ret = 0;

  if (test_subcase("tx bump small", test_txbump_small, NULL))
    ret = 1;

  if (test_subcase("tx bump full", test_txbump_full, NULL))
    ret = 1;

  if (test_subcase("tx bump too long", test_txbump_toolong, NULL))
    ret = 1;

  if (test_subcase("rx bump too long", test_rxbump_toolong, NULL))
    ret = 1;

  if (test_subcase("rx bump fc reopen no tx", test_rxbump_fc_reopen_notx, NULL))
    ret = 1;

  if (test_subcase("rx bump fc reopen tx", test_rxbump_fc_reopen_tx, NULL))
    ret = 1;

  if (test_subcase("rx bump fc reopen deadlock",
        test_rxbump_fc_reopen_deadlock, NULL))
    ret = 1;

  return ret;
}