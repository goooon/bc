#include "./visGpsTask.h"
#include "../../inc/Application.h"
#include "../../inc/Mqtt.h"
#include "../BCMessage.h"

#if BC_TARGET == BC_TARGET_ANDROID
#include "../../android/android_vicp.h"
#endif

Task* visGpsTask::Create(u32 appId)
{
	return bc_new visGpsTask(appId);
}

visGpsTask::visGpsTask(u32 appId) :Task(appId, true) {}

void visGpsTask::printGps(bcp_packet_t *pkg)
{
	bcp_message_t *m = NULL;
	bcp_element_t *e = NULL;

	while ((m = bcp_next_message(pkg, m)) != NULL) {
		e = NULL;
		while ((e = bcp_next_element(m, e)) != NULL) {
			printf("%s", e->data);
#if BC_TARGET == BC_TARGET_ANDROID
			android_vicp_notify(m->hdr.id, e->data, e->len);
#endif
		}
	}
}

void visGpsTask::doTask()
{
	for (;;) {
		ThreadEvent::WaitResult wr = waitForEvent(500);
		if (wr == ThreadEvent::EventOk) {
			MessageQueue::Args args;
			if (msgQueue.out(args)) {
				if (args.e == AppEvent::PackageArrived) {
					bcp_packet_t *p = (bcp_packet_t*)args.data;
					if (p) {
						printGps(p);
						bcp_packet_destroy(p);
					}
				}
			}
		}
	}
}