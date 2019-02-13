/* I must determine which of the two subdevices generated the interrupt */
                if ((deviceRegister->term.transm_status & DEV_TERM_STATUS) == DEV_TTRS_S_CHARTRSM) /* Successful
                                                                                 transmission */
                    which = TRANSM;
                else if ((deviceRegister->term.recv_status & DEV_TERM_STATUS) == DEV_TRCV_S_CHARRECV) /* Successful
                                                                                    receipt */
                    which = RECV;
                else
                {
                    tprint("Failed both transmission and receipt (MESSAGE BY KERNEL, NOT P2TEST)");
                    PANIC();
                }
                p = headBlocked(&terminals[j][which]);
                if (p == NULL)
                {
                    	//tprint("No device blocked on semaphore (MESSAGE BY KERNEL, NOT P2TEST)\n");
                    	//PANIC();
			//dispatch(NULL);
		    	still_soft_blocked=1;			//se uno fa tprint in questo modo non dovrebbe crashare
                }else
		        if (which == TRANSM) /* Returning the status of the device */
		            p->p_s.a1 = deviceRegister->term.transm_status; 
		        else if (which == RECV)
		            p->p_s.a1 = deviceRegister->term.recv_status;
		        V(&terminals[j][which],(state_t*)INT_OLDAREA);
		        if (which == TRANSM)
		            deviceRegister->term.transm_command = DEV_C_ACK; /* acknowledging the interrupt */
		        else if (which == RECV)
		            deviceRegister->term.recv_command = DEV_C_ACK; /* acknowledging the interrupt */