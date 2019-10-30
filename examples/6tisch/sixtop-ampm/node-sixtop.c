/*
 * Copyright (c) 2015, SICS Swedish ICT.
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. Neither the name of the Institute nor the names of its contributors
 *    may be used to endorse or promote products derived from this software
 *    without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE INSTITUTE AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE INSTITUTE OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 *
 */
/**
 * \file
 *         To test 6P transaction on a RPL+TSCH network
 *
 * \author
 *         Simon Duquennoy <simonduq@sics.se>
 *         Shalu R <shalur@cdac.in>
 *         Lijo Thomas <lijo@cdac.in>
 */

#include "contiki.h"
#include "sys/node-id.h"
#include "sys/log.h"
#include "net/ipv6/uip-ds6-route.h"
#include "net/mac/tsch/tsch.h"
#include "net/mac/tsch/sixtop/sixtop.h"
#include "net/routing/routing.h"
#include "net/netstack.h"
#include "net/ipv6/simple-udp.h"
#include "random.h"
#include "net/ipv6/uip.h"

#include "sf-simple.h"

#define DEBUG DEBUG_PRINT
#include "net/ipv6/uip-debug.h"


#define LOG_MODULE "App"
#define LOG_LEVEL LOG_LEVEL_INFO

#define WITH_SERVER_REPLY  0
#define UDP_CLIENT_PORT	8765
#define UDP_SERVER_PORT	5678
#define AMPM_CLIENT_PORT	8766
#define AMPM_SERVER_PORT	5679
#define INITIAL_DELAY	 	(60 * CLOCK_SECOND)
#define MAX_SEND_INTERVAL	 	(CLOCK_SECOND*1)
#define MIN_SEND_INTERVAL	 	(CLOCK_SECOND/10)
#define SIXTOP_CHECK_DELAY	 	10 // In order to add variation/randomness
#define MAX_PAYLOAD	56		//64
#define MIN_PAYLOAD	36		//32

static struct simple_udp_connection udp_conn;
static struct simple_udp_connection udp_conn_ampm;

/*---------------------------------------------------------------------------*/
PROCESS(node_process, "RPL Node");
AUTOSTART_PROCESSES(&node_process);

/*---------------------------------------------------------------------------*/
static void
udp_rx_callback(struct simple_udp_connection *c,
         const uip_ipaddr_t *sender_addr,
         uint16_t sender_port,
         const uip_ipaddr_t *receiver_addr,
         uint16_t receiver_port,
         const uint8_t *data,
         uint16_t datalen)
{

  LOG_INFO("A-K: Received message with length '%u' from ", datalen);
  LOG_INFO_6ADDR(sender_addr);
  LOG_INFO_("\n");
}

static void
ampm_rx_callback(struct simple_udp_connection *c,
         const uip_ipaddr_t *sender_addr,
         uint16_t sender_port,
         const uip_ipaddr_t *receiver_addr,
         uint16_t receiver_port,
         const uint8_t *data,
         uint16_t datalen)
{
	LOG_INFO((char*)data);
}


/*---------------------------------------------------------------------------*/
PROCESS_THREAD(node_process, ev, data)
{
  static int is_coordinator;
  static int is_source,is_dest,is_critical;
  static int added_num_of_links = 0;
  static struct etimer et;
  struct tsch_neighbor *n;
  static unsigned count;
  static char str[MAX_PAYLOAD];
  uip_ipaddr_t dest_ipaddr;
  uip_ipaddr_t br_ipaddr;
  
  PROCESS_BEGIN();

 /* Initialize UDP connection */
  simple_udp_register(&udp_conn, UDP_CLIENT_PORT, NULL,
                      UDP_SERVER_PORT, udp_rx_callback);
                      
  simple_udp_register(&udp_conn_ampm, AMPM_CLIENT_PORT, NULL,
                      AMPM_SERVER_PORT, ampm_rx_callback);

  is_coordinator = 0;

#if CONTIKI_TARGET_COOJA
  is_coordinator = (node_id == 1);
  is_source = (node_id == 3 || node_id == 4);
  is_critical = (node_id == 3);
  is_dest = (node_id == 6);
#endif

  if(is_coordinator) {
    NETSTACK_ROUTING.root_start();
		  /* Initialize UDP connection */
		  simple_udp_register(&udp_conn, UDP_SERVER_PORT, NULL,
						  UDP_CLIENT_PORT, udp_rx_callback);
		  /* Initialize UDP connection */
		  simple_udp_register(&udp_conn_ampm, AMPM_SERVER_PORT, NULL,
						  AMPM_CLIENT_PORT, ampm_rx_callback);
  }

  NETSTACK_MAC.on();
  sixtop_add_sf(&sf_simple_driver);


    if(!is_coordinator) {
	  etimer_set(&et, INITIAL_DELAY+((random_rand()%SIXTOP_CHECK_DELAY)*CLOCK_SECOND));
		if(is_dest){
			  /* Initialize UDP connection */
			  simple_udp_register(&udp_conn, UDP_SERVER_PORT, NULL,
							  UDP_CLIENT_PORT, udp_rx_callback);
		}


	  while(1) {
	    PROCESS_YIELD_UNTIL(etimer_expired(&et));
			etimer_reset(&et);
			
			/* Get time-source neighbor */
			n = tsch_queue_get_time_source();

			if(n!=NULL){
				  if((added_num_of_links < 1)) {
						printf("App : Add a link\n");
						if(sf_simple_add_links(&n->addr, 1)>=0){
							added_num_of_links++;
						}
						etimer_set(&et, (random_rand()%SIXTOP_CHECK_DELAY)*CLOCK_SECOND);
				  } else if(added_num_of_links > 1) {
						printf("App : Delete a link\n");
						if(sf_simple_remove_links(&n->addr)>=0){
							added_num_of_links=added_num_of_links-1;
						}
						etimer_set(&et, (random_rand()%SIXTOP_CHECK_DELAY)*CLOCK_SECOND);
				  }
				  else{
					if(is_source){
						if(NETSTACK_ROUTING.node_is_reachable() && NETSTACK_ROUTING.get_root_ipaddr(&br_ipaddr)) {
						  int message_length=(MIN_PAYLOAD + random_rand() % (MAX_PAYLOAD - MIN_PAYLOAD))%MAX_PAYLOAD;
						  
						  if(is_critical){
							//uip_ip6addr(&dest_ipaddr, 0xfd00, 0x0, 0x0, 0x0, 0x206, 0x6, 0x6, 0x6);
							uip_ip6addr(&dest_ipaddr, 0xfd00, 0x0, 0x0, 0x0, 0x212, 0x4B00, 0x1003, 0x5436);
							is_udp_critical=1;
						  }
						  else{
							uip_ip6addr(&dest_ipaddr, 0xfd00, 0x0, 0x0, 0x0, 0x201, 0x1, 0x1, 0x1);  
						  }
						  LOG_INFO("A-K: Sending request %u to ", count);
						  LOG_INFO_6ADDR(&dest_ipaddr);
						  LOG_INFO_("with length %u\n",message_length);

						  snprintf(str, sizeof(str), "hello %d", count);

						  simple_udp_sendto(&udp_conn, str, message_length, &dest_ipaddr);
						  count++;
						} else {
						  LOG_INFO("Not reachable yet\n");
						}
						
						if(ampm_to_report==1){
							if(NETSTACK_ROUTING.node_is_reachable() && NETSTACK_ROUTING.get_root_ipaddr(&br_ipaddr)) {						
							  LOG_INFO("A-K: Sending AMPM Report\n");
							  is_udp_critical=0;
							  simple_udp_sendto(&udp_conn_ampm, ampm_report_message, 80, &br_ipaddr);
							  ampm_to_report=0;
							} else {
							  LOG_INFO("A-K: Could not send AMPM Report: Not reachable yet\n");
							}
						}
						
						/* Add some jitter */
						//etimer_set(&et, SEND_INTERVAL - CLOCK_SECOND + (random_rand() % (2 * CLOCK_SECOND)));
						etimer_set(&et, MIN_SEND_INTERVAL + (random_rand() % MAX_SEND_INTERVAL));
					}
					else{
						if(ampm_to_report==1){
							if(NETSTACK_ROUTING.node_is_reachable() && NETSTACK_ROUTING.get_root_ipaddr(&br_ipaddr)) {						
							  LOG_INFO("A-K: Sending AMPM Report\n");
							  is_udp_critical=0;
							  simple_udp_sendto(&udp_conn_ampm, ampm_report_message, 80, &br_ipaddr);
							  ampm_to_report=0;
							} else {
							  LOG_INFO("A-K: Could not send AMPM Report: Not reachable yet\n");
							}
						}
						etimer_set(&et, CLOCK_SECOND);
					}

				}
			}
		    else{
				etimer_set(&et, (random_rand()%SIXTOP_CHECK_DELAY)*CLOCK_SECOND);
	        }
						
	     }
	  
		}
    else{
			etimer_set(&et, INITIAL_DELAY+((random_rand()%SIXTOP_CHECK_DELAY)*CLOCK_SECOND));
			while(1) {
				PROCESS_YIELD_UNTIL(etimer_expired(&et));
				etimer_reset(&et);	
				if(ampm_to_report==1){
						LOG_INFO(ampm_report_message);
						ampm_to_report=0;
				}
				etimer_set(&et, CLOCK_SECOND);	
			}  
						  
						  
     }

  PROCESS_END();
}
/*---------------------------------------------------------------------------*/

