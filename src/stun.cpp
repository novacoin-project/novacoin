/*
 * Get External IP address by STUN protocol
 *
 * Based on project Minimalistic STUN client "ministun"
 * https://code.google.com/p/ministun/
 *
 * This program is free software, distributed under the terms of
 * the GNU General Public License Version 2. See the LICENSE file
 * at the top of the source tree.
 *
 * STUN is described in RFC3489 and it is based on the exchange
 * of UDP packets between a client and one or more servers to
 * determine the externally visible address (and port) of the client
 * once it has gone through the NAT boxes that connect it to the
 * outside.
 * The simplest request packet is just the header defined in
 * struct stun_header, and from the response we may just look at
 * one attribute, STUN_MAPPED_ADDRESS, that we find in the response.
 * By doing more transactions with different server addresses we
 * may determine more about the behaviour of the NAT boxes, of
 * course - the details are in the RFC.
 *
 * All STUN packets start with a simple header made of a type,
 * length (excluding the header) and a 16-byte random transaction id.
 * Following the header we may have zero or more attributes, each
 * structured as a type, length and a value (whose format depends
 * on the type, but often contains addresses).
 * Of course all fields are in network format.
 */

#include <stdio.h>
#include <inttypes.h>
#include <limits>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#ifdef WIN32
#include <winsock2.h>
#else
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#endif
#ifndef WIN32
#include <unistd.h>
#endif
#include <time.h>
#include <errno.h>

#include "ministun.h"
#include "netbase.h"

extern int GetRandInt(int nMax);
extern uint64_t GetRand(uint64_t nMax);

/*---------------------------------------------------------------------*/

struct StunSrv {
    char     name[36];
    uint16_t port;
};

/*---------------------------------------------------------------------*/
// STUN server list from 2019-06-27
static const int StunSrvListQty = 619; // Must be PRIME!!!!!

static struct StunSrv StunSrvList[619] = {
  {"iphone-stun.strato-iphone.de",	3478},
  {"stun.1-voip.com",	3478},
  {"stun.12connect.com",	3478},
  {"stun.12voip.com",	3478},
  {"stun.1531.ru",	3478},
  {"stun.1cbit.ru",	3478},
  {"stun.1und1.de",	3478},
  {"stun.3cx.com",	3478},
  {"stun.3deluxe.de",	3478},
  {"stun.3wayint.com",	3478},
  {"stun.5sn.com",	3478},
  {"stun.a-mm.tv",	3478},
  {"stun.aa.net.uk",	3478},
  {"stun.aaisp.co.uk",	3478},
  {"stun.ab-hamm.de",	3478},
  {"stun.aceweb.com",	3478},
  {"stun.acquageraci.it",	3478},
  {"stun.acrobits.cz",	3478},
  {"stun.acronis.com",	3478},
  {"stun.actionvoip.com",	3478},
  {"stun.adafrance.ru",	3478},
  {"stun.advfn.com",	3478},
  {"stun.aeta-audio.com",	3478},
  {"stun.aeta.com",	3478},
  {"stun.alberon.cz",	3478},
  {"stun.allflac.com",	3478},
  {"stun.alphacron.de",	3478},
  {"stun.alpirsbacher.de",	3478},
  {"stun.anlx.net",	3478},
  {"stun.ansur.no",	3478},
  {"stun.antisip.com",	3478},
  {"stun.arkh-edu.ru",	3478},
  {"stun.artrage.com",	3478},
  {"stun.atagverwarming.nl",	3478},
  {"stun.atmapro.ru",	3478},
  {"stun.autosystem.com",	3478},
  {"stun.avigora.com",	3478},
  {"stun.avigora.fr",	3478},
  {"stun.avoxi.com",	3478},
  {"stun.axeos.nl",	3478},
  {"stun.axialys.net",	3478},
  {"stun.azurecoast.com",	3478},
  {"stun.b2b2c.ca",	3478},
  {"stun.babelforce.com",	3478},
  {"stun.bahnhof.net",	3478},
  {"stun.baltmannsweiler.de",	3478},
  {"stun.bandyer.com",	3478},
  {"stun.barbaratabita.it",	3478},
  {"stun.bau-ha.us",	3478},
  {"stun.bearstech.com",	3478},
  {"stun.beckhallmalham.com",	3478},
  {"stun.beebeetle.com",	3478},
  {"stun.bekkurso.info",	3478},
  {"stun.bergophor.de",	3478},
  {"stun.bernardoprovenzano.net",	3478},
  {"stun.bernies-bergwelt.com",	3478},
  {"stun.bethesda.net",	3478},
  {"stun.biamp.com",	3478},
  {"stun.bimp.fr",	3478},
  {"stun.bitburger.de",	3478},
  {"stun.bluesip.net",	3478},
  {"stun.botonakis.com",	3478},
  {"stun.bridesbay.com",	3478},
  {"stun.budgetsip.com",	3478},
  {"stun.bultest.org",	3478},
  {"stun.business-isp.nl",	3478},
  {"stun.cablenet-as.net",	3478},
  {"stun.call.me",	3478},
  {"stun.callromania.ro",	3478},
  {"stun.callwithus.com",	3478},
  {"stun.canwe.ca",	3478},
  {"stun.careerarc.com",	3478},
  {"stun.carlovizzini.it",	3478},
  {"stun.cdosea.org",	3478},
  {"stun.cellmail.com",	3478},
  {"stun.chaosmos.de",	3478},
  {"stun.chatous.com",	3478},
  {"stun.cheapvoip.com",	3478},
  {"stun.chewinggum.nl",	3478},
  {"stun.cibercloud.com.br",	3478},
  {"stun.circledoo.com",	3478},
  {"stun.clickphone.ro",	3478},
  {"stun.cloopen.com",	3478},
  {"stun.club-galicia-bonn.de",	3478},
  {"stun.cnj.gov.br",	3478},
  {"stun.cnj.jus.br",	3478},
  {"stun.coffee-sen.com",	3478},
  {"stun.comrex.com",	3478},
  {"stun.comtube.com",	3478},
  {"stun.comtube.ru",	3478},
  {"stun.connecteddata.com",	3478},
  {"stun.cope.es",	3478},
  {"stun.counterpath.com",	3478},
  {"stun.counterpath.net",	3478},
  {"stun.cozy.org",	3478},
  {"stun.createweb.de",	3478},
  {"stun.crimeastar.net",	3478},
  {"stun.crononauta.com",	3478},
  {"stun.cros.net",	3478},
  {"stun.csforall.org",	3478},
  {"stun.ctafauni.it",	3478},
  {"stun.databay.de",	3478},
  {"stun.dcalling.de",	3478},
  {"stun.deepfinesse.com",	3478},
  {"stun.degaronline.com",	3478},
  {"stun.demos.ru",	3478},
  {"stun.demos.su",	3478},
  {"stun.deutscherskiverband.de",	3478},
  {"stun.diallog.com",	3478},
  {"stun.dincloud.com",	3478},
  {"stun.dls.net",	3478},
  {"stun.dokom.net",	3478},
  {"stun.dorfbrunnen.eu",	3478},
  {"stun.dorisgraf.de",	3478},
  {"stun.dowlatow.ru",	3478},
  {"stun.draci.info",	3478},
  {"stun.dreifaltigkeit-stralsund.de",	3478},
  {"stun.dsin-berufsschulen.de",	3478},
  {"stun.dsin-blog.de",	3478},
  {"stun.dukun.de",	3478},
  {"stun.dunyatelekom.com",	3478},
  {"stun.dus.net",	3478},
  {"stun.eaclipt.org",	3478},
  {"stun.easter-eggs.com",	3478},
  {"stun.easycall.pl",	3478},
  {"stun.easyvoip.com",	3478},
  {"stun.edwin-wiegele.at",	3478},
  {"stun.effexx.com",	3478},
  {"stun.einfachcallback.de",	3478},
  {"stun.ekir.de",	3478},
  {"stun.eleusi.com",	3478},
  {"stun.elitetele.com",	3478},
  {"stun.engineeredarts.co.uk",	3478},
  {"stun.eol.co.nz",	3478},
  {"stun.eoni.com",	3478},
  {"stun.epygi.com",	3478},
  {"stun.eurescom.de",	3478},
  {"stun.eurosys.be",	3478},
  {"stun.evolution-movement.com",	3478},
  {"stun.exoplatform.org",	3478},
  {"stun.expandable.io",	3478},
  {"stun.extrasaltmobile.com",	3478},
  {"stun.faceflow.com",	3478},
  {"stun.faelix.net",	3478},
  {"stun.fairytel.at",	3478},
  {"stun.faktortel.com.au",	3478},
  {"stun.fathomvoice.com",	3478},
  {"stun.fbsbx.com",	3478},
  {"stun.feuerwehrmuseum.at",	3478},
  {"stun.fiberpipe.net",	3478},
  {"stun.files.fm",	3478},
  {"stun.finbolt.eu",	3478},
  {"stun.finsterwalder.com",	3478},
  {"stun.fitauto.ru",	3478},
  {"stun.fixup.net",	3478},
  {"stun.fmo.de",	3478},
  {"stun.foad.me.uk",	3478},
  {"stun.fondazioneroccochinnici.it",	3478},
  {"stun.framasoft.org",	3478},
  {"stun.framatalk.org",	3478},
  {"stun.freecall.com",	3478},
  {"stun.freeswitch.org",	3478},
  {"stun.freevoipdeal.com",	3478},
  {"stun.frozenmountain.com",	3478},
  {"stun.funwithelectronics.com",	3478},
  {"stun.futurasp.es",	3478},
  {"stun.galeriemagnet.at",	3478},
  {"stun.galerievorspann.com",	3478},
  {"stun.geekyboo.com",	3478},
  {"stun.geesthacht.de",	3478},
  {"stun.genymotion.com",	3478},
  {"stun.geonet.ro",	3478},
  {"stun.getzoop.com",	3478},
  {"stun.gigaset.net",	3478},
  {"stun.gkarnet.org",	3478},
  {"stun.globalmeet.com",	3478},
  {"stun.gmx.de",	3478},
  {"stun.gmx.net",	3478},
  {"stun.gntel.nl",	3478},
  {"stun.godatenow.com",	3478},
  {"stun.goldener-internetpreis.de",	3478},
  {"stun.goldfish.ie",	3478},
  {"stun.gotye.com.cn",	3478},
  {"stun.graftlab.com",	3478},
  {"stun.gravitel.ru",	3478},
  {"stun.grazertrinkwasseringefahr.at",	3478},
  {"stun.groenewold-newmedia.de",	3478},
  {"stun.gruene.at",	3478},
  {"stun.guenzburg.de",	3478},
  {"stun.gulfsip.com",	3478},
  {"stun.h4v.eu",	3478},
  {"stun.hacknology.de",	3478},
  {"stun.halonet.pl",	3478},
  {"stun.hanacke.net",	3478},
  {"stun.hardt-ware.de",	3478},
  {"stun.healthtap.com",	3478},
  {"stun.heeds.eu",	3478},
  {"stun.hicare.net",	3478},
  {"stun.hide.me",	3478},
  {"stun.highfidelity.io",	3478},
  {"stun.hinet.net",	3478},
  {"stun.histocaffe.com",	3478},
  {"stun.hitv.com",	3478},
  {"stun.hobby-drechselei.de",	3478},
  {"stun.hoiio.com",	3478},
  {"stun.hosteurope.de",	3478},
  {"stun.hot-chilli.net",	3478},
  {"stun.hotelparadisebeach.it",	3478},
  {"stun.ica-net.it",	3478},
  {"stun.ideasip.com",	3478},
  {"stun.ihk.cn",	3478},
  {"stun.imafex.sk",	3478},
  {"stun.imp.ch",	3478},
  {"stun.ines-seidel.de",	3478},
  {"stun.infra.net",	3478},
  {"stun.innotel.com.au",	3478},
  {"stun.innovaphone.com",	3478},
  {"stun.innovaphone.de",	3478},
  {"stun.instantteleseminar.com",	3478},
  {"stun.internetcalls.com",	3478},
  {"stun.interplanet.it",	3478},
  {"stun.intervoip.com",	3478},
  {"stun.invaluable.com",	3478},
  {"stun.ippi.com",	3478},
  {"stun.ippi.fr",	3478},
  {"stun.ipshka.com",	3478},
  {"stun.irishvoip.com",	3478},
  {"stun.isp.net.au",	3478},
  {"stun.issabel.org",	3478},
  {"stun.istitutogramscisiciliano.it",	3478},
  {"stun.it1.hr",	3478},
  {"stun.ivao.aero",	3478},
  {"stun.ivao.co.uk",	3478},
  {"stun.ivi.at",	3478},
  {"stun.ixc.ua",	3478},
  {"stun.jabber.dk",	3478},
  {"stun.jabbim.cz",	3478},
  {"stun.janmedia.pl",	3478},
  {"stun.jay.net",	3478},
  {"stun.jowisoftware.de",	3478},
  {"stun.jpluso.cz",	3478},
  {"stun.julienschmidt.com",	3478},
  {"stun.julosoft.net",	3478},
  {"stun.jumblo.com",	3478},
  {"stun.junet.se",	3478},
  {"stun.justvoip.com",	3478},
  {"stun.kanojo.de",	3478},
  {"stun.kaseya.com",	3478},
  {"stun.katholischekirche-ruegen.de",	3478},
  {"stun.kaznpu.kz",	3478},
  {"stun.kedr.io",	3478},
  {"stun.kendama.ru",	3478},
  {"stun.kiesler.at",	3478},
  {"stun.kitamaebune.com",	3478},
  {"stun.ko2100.at",	3478},
  {"stun.komsa.de",	3478},
  {"stun.kore.com",	3478},
  {"stun.kostenloses-forum.com",	3478},
  {"stun.kotter.net",	3478},
  {"stun.kreis-bergstrasse.de",	3478},
  {"stun.l.google.com",	19302},
  {"stun.l.google.com",	19305},
  {"stun.labs.net",	3478},
  {"stun.lacompagnieducode.org",	3478},
  {"stun.ladridiricette.it",	3478},
  {"stun.landvast.nl",	3478},
  {"stun.ldiglobal.org",	3478},
  {"stun.le-space.de",	3478},
  {"stun.lebendigefluesse.at",	3478},
  {"stun.leibergmbh.de",	3478},
  {"stun.leonde.org",	3478},
  {"stun.lerros.com",	3478},
  {"stun.leucotron.com.br",	3478},
  {"stun.levigo.de",	3478},
  {"stun.lindab.com",	3478},
  {"stun.lineaencasa.com",	3478},
  {"stun.linphone.org",	3478},
  {"stun.linuxtrent.it",	3478},
  {"stun.liveo.fr",	3478},
  {"stun.lleida.net",	3478},
  {"stun.localphone.com",	3478},
  {"stun.logenex.com",	3478},
  {"stun.logic.ky",	3478},
  {"stun.londonweb.net",	3478},
  {"stun.lovense.com",	3478},
  {"stun.lowratevoip.com",	3478},
  {"stun.lundimatin.fr",	3478},
  {"stun.m-online.net",	3478},
  {"stun.madavi.de",	3478},
  {"stun.mage.com.vn",	3478},
  {"stun.magnocall.com",	3478},
  {"stun.magyarphone.eu",	3478},
  {"stun.malemotion.com",	3478},
  {"stun.mangotele.com",	3478},
  {"stun.marble.io",	3478},
  {"stun.marcelproust.it",	3478},
  {"stun.mda.gov.br",	3478},
  {"stun.mdaemon.com",	3478},
  {"stun.medvc.eu",	3478},
  {"stun.meetangee.com",	3478},
  {"stun.meetwife.com",	3478},
  {"stun.megakosmos.com.br",	3478},
  {"stun.megatel.si",	3478},
  {"stun.meowsbox.com",	3478},
  {"stun.millenniumarts.org",	3478},
  {"stun.mit.de",	3478},
  {"stun.miwifi.com",	3478},
  {"stun.mixvoip.com",	3478},
  {"stun.mls.com.br",	3478},
  {"stun.mobile-italia.com",	3478},
  {"stun.modulus.gr",	3478},
  {"stun.mondiaspora.net",	3478},
  {"stun.mondiaspora.org",	3478},
  {"stun.moonlight-stream.org",	3478},
  {"stun.mot-net.com",	3478},
  {"stun.mrmondialisation.org",	3478},
  {"stun.muoversi.net",	3478},
  {"stun.museumsguetesiegel.at",	3478},
  {"stun.myfreecams.com",	3478},
  {"stun.myfriends.ru",	3478},
  {"stun.myhowto.org",	3478},
  {"stun.mylio.com",	3478},
  {"stun.myspeciality.com",	3478},
  {"stun.myvoipapp.com",	3478},
  {"stun.myvoiptraffic.com",	3478},
  {"stun.mywatson.it",	3478},
  {"stun.nabto.com",	3478},
  {"stun.nanocosmos.de",	3478},
  {"stun.naturfakta.com",	3478},
  {"stun.nautile.nc",	3478},
  {"stun.ncic.com",	3478},
  {"stun.neomedia.it",	3478},
  {"stun.nerathor.com",	3478},
  {"stun.net-mag.cz",	3478},
  {"stun.netappel.com",	3478},
  {"stun.newrocktech.com",	3478},
  {"stun.nexphone.ch",	3478},
  {"stun.next-gen.ro",	3478},
  {"stun.nextcloud.com",	3478},
  {"stun.nexxtmobile.de",	3478},
  {"stun.nfon.net",	3478},
  {"stun.nicokrause.com",	3478},
  {"stun.nieuwland.cc",	3478},
  {"stun.niksteknoloji.com",	3478},
  {"stun.nonoh.net",	3478},
  {"stun.nosapps.com",	3478},
  {"stun.nottingham.ac.uk",	3478},
  {"stun.nova.is",	3478},
  {"stun.nstelcom.com",	3478},
  {"stun.obovsem.com",	3478},
  {"stun.odr.de",	3478},
  {"stun.officinabit.com",	3478},
  {"stun.olimontel.it",	3478},
  {"stun.omnitor.se",	3478},
  {"stun.oncloud7.ch",	3478},
  {"stun.onesuite.com",	3478},
  {"stun.onthenet.com.au",	3478},
  {"stun.ooma.com",	3478},
  {"stun.openjobs.hu",	3478},
  {"stun.openvoip.it",	3478},
  {"stun.optdyn.com",	3478},
  {"stun.orszaczky.com",	3478},
  {"stun.ortopediacoam.it",	3478},
  {"stun.osbid.cz",	3478},
  {"stun.otos.pl",	3478},
  {"stun.pados.hu",	3478},
  {"stun.palava.tv",	3478},
  {"stun.parcodeinebrodi.it",	3478},
  {"stun.pbo.ru",	3478},
  {"stun.peeters.com",	3478},
  {"stun.peethultra.be",	3478},
  {"stun.penserpouragir.org",	3478},
  {"stun.peoplefone.ch",	3478},
  {"stun.personal-voip.de",	3478},
  {"stun.phone.com",	3478},
  {"stun.photojim.ca",	3478},
  {"stun.phytter.com",	3478},
  {"stun.piratecinema.org",	3478},
  {"stun.piratenbrandenburg.de",	3478},
  {"stun.pjsip.org",	3478},
  {"stun.planetarium.com.br",	3478},
  {"stun.plexicomm.net",	3478},
  {"stun.pobeda-club.ru",	3478},
  {"stun.poetamatusel.org",	3478},
  {"stun.poivy.com",	3478},
  {"stun.potsdamvibes.de",	3478},
  {"stun.powervoip.com",	3478},
  {"stun.ppdi.com",	3478},
  {"stun.provectio.fr",	3478},
  {"stun.pruefplan.com",	3478},
  {"stun.psipsi.com.br",	3478},
  {"stun.pure-ip.com",	3478},
  {"stun.qbictechnology.com",	3478},
  {"stun.qcol.net",	3478},
  {"stun.qq.com",	3478},
  {"stun.qwant.com",	3478},
  {"stun.rackco.com",	3478},
  {"stun.racknine.net",	3478},
  {"stun.radiojar.com",	3478},
  {"stun.ransquawk.com",	3478},
  {"stun.readyforsky.com",	3478},
  {"stun.realgarant.com",	3478},
  {"stun.redworks.nl",	3478},
  {"stun.revreso.de",	3478},
  {"stun.ringostat.com",	3478},
  {"stun.ringvoz.com",	3478},
  {"stun.rmf.pl",	3478},
  {"stun.robocup.at",	3478},
  {"stun.rockenstein.de",	3478},
  {"stun.rolmail.net",	3478},
  {"stun.romaaeterna.nl",	3478},
  {"stun.romancecompass.com",	3478},
  {"stun.root-1.de",	3478},
  {"stun.ru-brides.com",	3478},
  {"stun.rudtp.ru",	3478},
  {"stun.rynga.com",	3478},
  {"stun.sacko.com.au",	3478},
  {"stun.samy.pl",	3478},
  {"stun.saooti.com",	3478},
  {"stun.savemgo.com",	3478},
  {"stun.scalix.com",	3478},
  {"stun.schlund.de",	3478},
  {"stun.schoeffel.de",	3478},
  {"stun.schulinformatik.at",	3478},
  {"stun.selasky.org",	3478},
  {"stun.semiocast.com",	3478},
  {"stun.senstar.com",	3478},
  {"stun.sewan.fr",	3478},
  {"stun.sg-slope.com",	3478},
  {"stun.shadrinsk.net",	3478},
  {"stun.sharpbai.com",	3478},
  {"stun.shy.cz",	3478},
  {"stun.siedle.com",	3478},
  {"stun.sigmavoip.com",	3478},
  {"stun.signalwire.com",	3478},
  {"stun.simlar.org",	3478},
  {"stun.sip.us",	3478},
  {"stun.sipdiscount.com",	3478},
  {"stun.sipgate.net",	10000},
  {"stun.sipgate.net",	3478},
  {"stun.sipglobalphone.com",	3478},
  {"stun.siplogin.de",	3478},
  {"stun.sipnet.com",	3478},
  {"stun.siportal.it",	3478},
  {"stun.sippeer.dk",	3478},
  {"stun.sipthor.net",	3478},
  {"stun.siptraffic.com",	3478},
  {"stun.siptrunk.com",	3478},
  {"stun.sipy.cz",	3478},
  {"stun.sius.com",	3478},
  {"stun.sketch.io",	3478},
  {"stun.skrumble.com",	3478},
  {"stun.sky.od.ua",	3478},
  {"stun.skydrone.aero",	3478},
  {"stun.slackline.at",	3478},
  {"stun.sma.de",	3478},
  {"stun.smartvoip.com",	3478},
  {"stun.smdr.ru",	3478},
  {"stun.smsdiscount.com",	3478},
  {"stun.smslisto.com",	3478},
  {"stun.soho66.co.uk",	3478},
  {"stun.sokoll.com",	3478},
  {"stun.solcon.nl",	3478},
  {"stun.solnet.ch",	3478},
  {"stun.solomo.de",	3478},
  {"stun.sonetel.com",	3478},
  {"stun.sonetel.net",	3478},
  {"stun.sovtest.ru",	3478},
  {"stun.sparvoip.de",	3478},
  {"stun.speedy.com.ar",	3478},
  {"stun.splicecom.com",	3478},
  {"stun.spoiltheprincess.com",	3478},
  {"stun.spreed.me",	3478},
  {"stun.srce.hr",	3478},
  {"stun.stadtwerke-eutin.de",	3478},
  {"stun.stbuehler.de",	3478},
  {"stun.steinbeis-smi.de",	3478},
  {"stun.stochastix.de",	3478},
  {"stun.stratusvideo.com",	3478},
  {"stun.streamix.live",	3478},
  {"stun.streamnow.ch",	3478},
  {"stun.studio-link.de",	3478},
  {"stun.studio71.it",	3478},
  {"stun.stunprotocol.org",	3478},
  {"stun.stuttgart-ix.de",	3478},
  {"stun.surjaring.it",	3478},
  {"stun.surrealnetworks.com",	3478},
  {"stun.swissquote.com",	3478},
  {"stun.swrag.de",	3478},
  {"stun.sylaps.com",	3478},
  {"stun.symonics.com",	3478},
  {"stun.syncthing.net",	3478},
  {"stun.synergiejobs.be",	3478},
  {"stun.syrex.co.za",	3478},
  {"stun.t-online.de",	3478},
  {"stun.talk.by",	3478},
  {"stun.talkho.com",	3478},
  {"stun.talks.by",	3478},
  {"stun.taxsee.com",	3478},
  {"stun.teamfon.de",	3478},
  {"stun.technosens.fr",	3478},
  {"stun.teconisy.com",	3478},
  {"stun.tel.lu",	3478},
  {"stun.tel2.co.uk",	3478},
  {"stun.telbo.com",	3478},
  {"stun.tele2.net",	3478},
  {"stun.telemar.it",	3478},
  {"stun.teliax.com",	3478},
  {"stun.telnyx.com",	3478},
  {"stun.telonline.com",	3478},
  {"stun.telviva.com",	3478},
  {"stun.telzio.com",	3478},
  {"stun.testreach.com",	3478},
  {"stun.textz.com",	3478},
  {"stun.thebrassgroup.it",	3478},
  {"stun.thfree.ru",	3478},
  {"stun.thinkrosystem.com",	3478},
  {"stun.threema.ch",	3478},
  {"stun.tichiamo.it",	3478},
  {"stun.tng.de",	3478},
  {"stun.totalcom.info",	3478},
  {"stun.touchcast.com",	3478},
  {"stun.training-online.eu",	3478},
  {"stun.trainingspace.online",	3478},
  {"stun.trivenet.it",	3478},
  {"stun.ttmath.org",	3478},
  {"stun.tula.nu",	3478},
  {"stun.twt.it",	3478},
  {"stun.uabrides.com",	3478},
  {"stun.uavia.eu",	3478},
  {"stun.uiltucssicilia.it",	3478},
  {"stun.ukh.de",	3478},
  {"stun.uls.co.za",	3478},
  {"stun.unseen.is",	3478},
  {"stun.url.net.au",	3478},
  {"stun.vadacom.co.nz",	3478},
  {"stun.var6.cn",	3478},
  {"stun.vavadating.com",	3478},
  {"stun.verbo.be",	3478},
  {"stun.viagenie.ca",	3478},
  {"stun.villapalagonia.it",	3478},
  {"stun.vincross.com",	3478},
  {"stun.viptel.sk",	3478},
  {"stun.visocon.com",	3478},
  {"stun.viva.gr",	3478},
  {"stun.vivox.com",	3478},
  {"stun.vnc.biz",	3478},
  {"stun.vo.lu",	3478},
  {"stun.voicetech.se",	3478},
  {"stun.voicetrading.com",	3478},
  {"stun.voip.aebc.com",	3478},
  {"stun.voip.blackberry.com",	3478},
  {"stun.voip.eutelia.it",	3478},
  {"stun.voiparound.com",	3478},
  {"stun.voipblast.com",	3478},
  {"stun.voipbuster.com",	3478},
  {"stun.voipbusterpro.com",	3478},
  {"stun.voipcheap.com",	3478},
  {"stun.voipconnect.com",	3478},
  {"stun.voipdiscount.com",	3478},
  {"stun.voipfibre.com",	3478},
  {"stun.voipgain.com",	3478},
  {"stun.voipgate.com",	3478},
  {"stun.voipgrid.nl",	3478},
  {"stun.voipia.net",	3478},
  {"stun.voipinfocenter.com",	3478},
  {"stun.voippro.com",	3478},
  {"stun.voipraider.com",	3478},
  {"stun.voiprakyat.or.id",	3478},
  {"stun.voipstreet.com",	3478},
  {"stun.voipstunt.com",	3478},
  {"stun.voipvoice.it",	3478},
  {"stun.voipwise.com",	3478},
  {"stun.voipxs.nl",	3478},
  {"stun.voipzoom.com",	3478},
  {"stun.vomessen.de",	3478},
  {"stun.voxox.com",	3478},
  {"stun.voys.nl",	3478},
  {"stun.vozelia.com",	3478},
  {"stun.voztele.com",	3478},
  {"stun.voztovoice.org",	3478},
  {"stun.waterpolopalermo.it",	3478},
  {"stun.wcoil.com",	3478},
  {"stun.webcalldirect.com",	3478},
  {"stun.webfreak.org",	3478},
  {"stun.webitel.ua",	3478},
  {"stun.webmatrix.com.br",	3478},
  {"stun.webscience-journal.net",	3478},
  {"stun.weepee.org",	3478},
  {"stun.wemag.com",	3478},
  {"stun.westtel.ky",	3478},
  {"stun.whc.net",	3478},
  {"stun.wia.cz",	3478},
  {"stun.wifirst.net",	3478},
  {"stun.wtfismyip.com",	3478},
  {"stun.wxnz.net",	3478},
  {"stun.xn----8sbcoa5btidn9i.xn--p1ai",	3478},
  {"stun.xten.com",	3478},
  {"stun.xtratelecom.es",	3478},
  {"stun.yangutu.com",	3478},
  {"stun.yesdates.com",	3478},
  {"stun.yeymo.com",	3478},
  {"stun.yollacalls.com",	3478},
  {"stun.yy.com",	3478},
  {"stun.zadarma.com",	3478},
  {"stun.zentauron.de",	3478},
  {"stun.zepter.ru",	3478},
  {"stun.zombiegrinder.com",	3478},
  {"stun.zottel.net",	3478},
  {"stun.zuckschwerdt.org",	3478},
  {"stun1.faktortel.com.au",	3478},
  {"stun1.l.google.com",	19302},
  {"stun1.l.google.com",	19305},
  {"stun2.l.google.com",	19302},
  {"stun2.l.google.com",	19305},
  {"stun3.l.google.com",	19302},
  {"stun3.l.google.com",	19305},
  {"stun4.l.google.com",	19302},
  {"stun4.l.google.com",	19305}
};


/* wrapper to send an STUN message */
static int stun_send(int s, struct sockaddr_in *dst, struct stun_header *resp)
{
    return sendto(s, (const char *)resp, ntohs(resp->msglen) + sizeof(*resp), 0,
                  (struct sockaddr *)dst, sizeof(*dst));
}

/* helper function to generate a random request id */
static uint64_t randfiller = GetRand(std::numeric_limits<uint64_t>::max());
static void stun_req_id(struct stun_header *req)
{
    const uint64_t *S_block = (const uint64_t *)StunSrvList;
    req->id.id[0] = GetRandInt(std::numeric_limits<int32_t>::max());
    req->id.id[1] = GetRandInt(std::numeric_limits<int32_t>::max());
    req->id.id[2] = GetRandInt(std::numeric_limits<int32_t>::max());
    req->id.id[3] = GetRandInt(std::numeric_limits<int32_t>::max());

    req->id.id[0] |= 0x55555555;
    req->id.id[1] &= 0x55555555;
    req->id.id[2] |= 0x55555555;
    req->id.id[3] &= 0x55555555;
    char x = 20;
    do {
        uint32_t s_elm = S_block[(uint8_t)randfiller];
        randfiller ^= (randfiller << 5) | (randfiller >> (64 - 5));
        randfiller += s_elm ^ x;
        req->id.id[x & 3] ^= randfiller + (randfiller >> 13);
    } while(--x);
}

/* callback type to be invoked on stun responses. */
typedef int (stun_cb_f)(struct stun_attr *attr, void *arg);

/* handle an incoming STUN message.
 *
 * Do some basic sanity checks on packet size and content,
 * try to extract a bit of information, and possibly reply.
 * At the moment this only processes BIND requests, and returns
 * the externally visible address of the request.
 * If a callback is specified, invoke it with the attribute.
 */
static int stun_handle_packet(int s, struct sockaddr_in *src,
                              unsigned char *data, size_t len, stun_cb_f *stun_cb, void *arg)
{
    struct stun_header *hdr = (struct stun_header *)data;
    struct stun_attr *attr;
    int ret = len;
    unsigned int x;

    /* On entry, 'len' is the length of the udp payload. After the
   * initial checks it becomes the size of unprocessed options,
   * while 'data' is advanced accordingly.
   */
    if (len < sizeof(struct stun_header))
        return -20;

    len -= sizeof(struct stun_header);
    data += sizeof(struct stun_header);
    x = ntohs(hdr->msglen); /* len as advertised in the message */
    if(x < len)
        len = x;

    while (len) {
        if (len < sizeof(struct stun_attr)) {
            ret = -21;
            break;
        }
        attr = (struct stun_attr *)data;
        /* compute total attribute length */
        x = ntohs(attr->len) + sizeof(struct stun_attr);
        if (x > len) {
            ret = -22;
            break;
        }
        stun_cb(attr, arg);
        //if (stun_process_attr(&st, attr)) {
        //  ret = -23;
        //  break;
        // }
        /* Clear attribute id: in case previous entry was a string,
     * this will act as the terminator for the string.
     */
        attr->attr = 0;
        data += x;
        len -= x;
    } // while
    /* Null terminate any string.
   * XXX NOTE, we write past the size of the buffer passed by the
   * caller, so this is potentially dangerous. The only thing that
   * saves us is that usually we read the incoming message in a
   * much larger buffer
   */
    *data = '\0';

    /* Now prepare to generate a reply, which at the moment is done
   * only for properly formed (len == 0) STUN_BINDREQ messages.
   */

    return ret;
}

/* Extract the STUN_MAPPED_ADDRESS from the stun response.
 * This is used as a callback for stun_handle_response
 * when called from stun_request.
 */
static int stun_get_mapped(struct stun_attr *attr, void *arg)
{
    struct stun_addr *addr = (struct stun_addr *)(attr + 1);
    struct sockaddr_in *sa = (struct sockaddr_in *)arg;

    if (ntohs(attr->attr) != STUN_MAPPED_ADDRESS || ntohs(attr->len) != 8)
        return 1; /* not us. */
    sa->sin_port = addr->port;
    sa->sin_addr.s_addr = addr->addr;
    return 0;
}

/*---------------------------------------------------------------------*/

static int StunRequest2(int sock, struct sockaddr_in *server, struct sockaddr_in *mapped) {

    struct stun_header *req;
    unsigned char reqdata[1024];

    req = (struct stun_header *)reqdata;
    stun_req_id(req);
    unsigned short reqlen = 0;
    req->msgtype = 0;
    req->msglen = 0;
    req->msglen = htons(reqlen);
    req->msgtype = htons(STUN_BINDREQ);

    unsigned char reply_buf[1024];
    fd_set rfds;
    struct timeval to = { STUN_TIMEOUT, 0 };
    struct sockaddr_in src;
#ifdef WIN32
    int srclen;
#else
    socklen_t srclen;
#endif

    int res = stun_send(sock, server, req);
    if(res < 0)
        return -10;
    FD_ZERO(&rfds);
    FD_SET(sock, &rfds);
    res = select(sock + 1, &rfds, NULL, NULL, &to);
    if (res <= 0)  /* timeout or error */
        return -11;
    memset(&src, 0, sizeof(src));
    srclen = sizeof(src);
    /* XXX pass -1 in the size, because stun_handle_packet might
   * write past the end of the buffer.
   */
    res = recvfrom(sock, (char *)reply_buf, sizeof(reply_buf) - 1,
                   0, (struct sockaddr *)&src, &srclen);
    if (res <= 0)
        return -12;
    memset(mapped, 0, sizeof(struct sockaddr_in));
    return stun_handle_packet(sock, &src, reply_buf, res, stun_get_mapped, mapped);
} // StunRequest2

/*---------------------------------------------------------------------*/
static int StunRequest(const char *host, uint16_t port, struct sockaddr_in *mapped) {
    struct hostent *hostinfo = gethostbyname(host);
    if(hostinfo == NULL)
        return -1;

    SOCKET sock = INVALID_SOCKET;
    struct sockaddr_in server, client;
    memset(&server, 0, sizeof(server));
    memset(&client, 0, sizeof(client));
    server.sin_family = client.sin_family = AF_INET;

    server.sin_addr = *(struct in_addr*) hostinfo->h_addr;
    server.sin_port = htons(port);

    sock = socket(AF_INET, SOCK_DGRAM, 0);
    if(sock == INVALID_SOCKET)
        return -2;

    client.sin_addr.s_addr = htonl(INADDR_ANY);

    int rc = -3;
    if(bind(sock, (struct sockaddr*)&client, sizeof(client)) >= 0)
        rc = StunRequest2(sock, &server, mapped);
    CloseSocket(sock);
    return rc;
} // StunRequest

/*---------------------------------------------------------------------*/
// Input: two random values (pos, step) for generate uniuqe way over server
// list
// Output: populate struct struct mapped
// Retval:

int GetExternalIPbySTUN(uint64_t rnd, struct sockaddr_in *mapped, const char **srv) {
    randfiller    = rnd;
    uint16_t pos  = rnd;
    uint16_t step;
    do {
        rnd = (rnd >> 8) | 0xff00000000000000LL;
        step = rnd % StunSrvListQty;
    } while(step == 0);

    uint16_t attempt;
    for(attempt = 1; attempt < StunSrvListQty * 2; attempt++) {
        pos = (pos + step) % StunSrvListQty;
        int rc = StunRequest(*srv = StunSrvList[pos].name, StunSrvList[pos].port, mapped);
        if(rc >= 0)
            return attempt;
        // fprintf(stderr, "Lookup: %s:%u\t%s\t%d\n", StunSrvList[pos].name,
        // StunSrvList[pos].port, inet_ntoa(mapped->sin_addr), rc);
    }
    return -1;
}
