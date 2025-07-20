/*
Copyright © 2025 Miranda NG team

This program is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 2 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#include "stdafx.h"

void CDeltaChatProto::HandleEvents()
{
	auto *emitter = dc_get_event_emitter(m_context);
	
	while (dc_event_t *event = dc_get_next_event(emitter)) {
		auto id = dc_event_get_id(event), i1 = dc_event_get_data1_int(event), i2 = dc_event_get_data2_int(event);
		auto s1 = dc_event_get_data1_str(event), s2 = dc_event_get_data2_str(event);
		debugLogA("DC event: %d (%d <%s>) (%d <%s>)", id, i1, s1, i2, s2);
		dc_event_unref(event);
	}

	dc_event_emitter_unref(emitter);
}
