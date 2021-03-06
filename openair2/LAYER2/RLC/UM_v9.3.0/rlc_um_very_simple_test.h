/*******************************************************************************
    OpenAirInterface
    Copyright(c) 1999 - 2014 Eurecom

    OpenAirInterface is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.


    OpenAirInterface is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with OpenAirInterface.The full GNU General Public License is
   included in this distribution in the file called "COPYING". If not,
   see <http://www.gnu.org/licenses/>.

  Contact Information
  OpenAirInterface Admin: openair_admin@eurecom.fr
  OpenAirInterface Tech : openair_tech@eurecom.fr
  OpenAirInterface Dev  : openair4g-devel@lists.eurecom.fr

  Address      : Eurecom, Campus SophiaTech, 450 Route des Chappes, CS 50193 - 06904 Biot Sophia Antipolis cedex, FRANCE

 *******************************************************************************/
/*! \file rlc_um_very_simple_test.h
* \brief This file defines the prototypes of the functions dealing with the sending of self generated packet for very basic test or debug of RLC or lower layers.
* \author GAUTHIER Lionel
* \date 2010-2011
* \version
* \note
* \bug
* \warning
*/
#    ifndef __RLC_UM_VERY_SIMPLE_TEST_H__
#        define __RLC_UM_VERY_SIMPLE_TEST_H__
//-----------------------------------------------------------------------------
#        include "rlc_um_entity.h"
#        include "mem_block.h"
#        include "rrm_config_structs.h"
#        include "rlc_um_structs.h"
#        include "rlc_um_constants.h"
//-----------------------------------------------------------------------------
#        ifdef RLC_UM_VERY_SIMPLE_TEST_C
#            define private_rlc_um_very_simple_test(x)    x
#            define protected_rlc_um_very_simple_test(x)  x
#            define public_rlc_um_very_simple_test(x)     x
#        else
#            ifdef RLC_UM_MODULE
#                define private_rlc_um_very_simple_test(x)
#                define protected_rlc_um_very_simple_test(x)  extern x
#                define public_rlc_um_very_simple_test(x)     extern x
#            else
#                define private_rlc_um_very_simple_test(x)
#                define protected_rlc_um_very_simple_test(x)
#                define public_rlc_um_very_simple_test(x)     extern x
#            endif
#        endif
#define RLC_UM_TEST_SDU_TYPE_TCPIP 0
#define RLC_UM_TEST_SDU_TYPE_VOIP  1
#define RLC_UM_TEST_SDU_TYPE_SMALL 2

#define tcip_sdu  "NOS TESTS MONTRENT QUE LE NOUVEAU TOSHIBA MK7559GSXP, UN DISQUE DUR DE 750 GO FONCTIONNANT AVEC DES SECTEURS DE 4 KO, EST AU MOINS AUSSI RAPIDE QUE SON PR??D??CESSEUR, LE MK6465GSX 640 GO, DANS TOUS LES BENCHMARKS TH??ORIQUES. SES PERFORMANCES EN E/S ET SON TEMPS D???ACC??S SONT COMPARABLES ET SON D??BIT R??EL EST M??ME NETTEMENT PLUS ??LEV??. SES R??SULTATS SOUS PCMARK VANTAGE, PAR CONTRE, SONT QUELQUE PEU MOINS BONS. DANS CE CAS, LEQUEL CHOISIR ? LES SCORES OBTENUS DANS LES TESTS TH??ORIQUES NOUS CONFIRMENT QUE LE NOUVEAU MOD??LE SE COMPORTE CONVENABLEMENT ?? MALGR?? ?? SES SECTEURS DE 4 KO ET QUE LA RAISON DU L??GER RECUL DE SES PERFORMANCES SOUS PCMARK VANTAGE SE TROUVE AILLEURS. L???ALIGNEMENT DES SECTEURS N???EST PAS NON PLUS EN CAUSE, ??TANT DONN?? QUE WINDOWS VISTA (NOTRE OS DE TEST) ET WINDOWS 7 EN TIENNENT COMPTE LORS DE LA CR??ATION DES PARTITIONS ??? CE QUE NOUS AVONS BIEN ENTENDU V??RIFI?? IND??PENDAMMENT.IL NOUS EST TOUTEFOIS IMPOSSIBLE DE CONTR??LER L???EX??CUTION ET L???ORGANISATION DE L?????CRITURE DES DONN??ES. PCMARK VANTAGE N???A EN EFFET JAMAIS ??T?? OPTIMIS?? POUR L?????CRITURE DE BLOCS DE DONN??ES DE GRANDE TAILLE ; DANS LA VIE R??ELLE, SI VOUS ??CRIVEZ SURTOUT DE GROS FICHIERS, LE NOUVEAU DISQUE DUR DE 750 GO VA S???AV??RER PLUS RAPIDE QUE LE 640 GO ET SURTOUT QUE LES R??SULTATS ENREGISTR??S DANS NOTRE BENCHMARK PCMARK, CAR SES SECTEURS DE 4 KO SERONT TOUJOURS PLUS PETITS QUE LES DONN??ES ?? ??CRIRE. LE PROBL??ME EST QU???AUSSI LONGTEMPS QUE LES APPLICATIONS CONTINUERONT ?? EFFECTUER DES DEMANDES D?????CRITURE EN MODE  512 OCTETS"

#define voip_sdu  "Nos tests montrent que le nouveau Toshiba MK7559GSXP, un disque dur de 750 Go"
#define very_small_sdu "NoS tEsTs MoNtReNt"

public_rlc_um_very_simple_test(void rlc_um_test_send_sdu (rlc_um_entity_t* rlcP,  uint32_t frame, unsigned int sdu_typeP));
#    endif
