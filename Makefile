# Custom ports collection makefile for:	netload
# Date created:				05 December 2006
# Whom:					George Crihan <geocrime@mail.ru>
#
# $UniConf: custom/netload/Makefile,v 0.1 2006/12/05 20:51:12 geocrime Exp $
#

VALID_CATEGORIES+= custom

PORTNAME=	netload
PORTVERSION=	2006.12.06
CATEGORIES=	custom
MASTER_SITES=	file://${PKGDIR}/src/
DISTFILES=	netload.tar.gz

MAINTAINER=	geocrime@mail.ru
COMMENT=	CUSTOM: Web-oriented utility to monitor average in/out traffic on a specified interface

#NO_WRKSUBDIR=	yes
NO_PACKAGE=	must be built for each system individually
WRKSRC=		${WRKDIR}/${PORTNAME}
FETCH_CMD=	/usr/bin/fetch -ARrl
PLIST_FILES=	bin/netload libexec/netload.ko etc/rc.d/netload.sh

#MAKEFILE=	${PKGDIR}/src/Makefile
MAKE_ENV=	PKGDIR="${PKGDIR}"

.include <bsd.port.pre.mk>
 
do-install:
	${INSTALL_PROGRAM} ${WRKSRC}/netload ${PREFIX}/bin/
	${INSTALL_DATA} ${WRKSRC}/netload.ko ${PREFIX}/libexec/

pre-install:
	@/usr/bin/awk -f /usr/adm/genconf/bin/m1.awk /usr/adm/genconf/master.conf ${WRKSRC}/netload.sh.in > ${WRKSRC}/netload.sh 

post-install:
	${INSTALL_SCRIPT} ${WRKSRC}/netload.sh ${PREFIX}/etc/rc.d/

.include <bsd.port.post.mk>
