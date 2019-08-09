/****************************************************************************
 *
 * Copyright 2019 Samsung Electronics All Rights Reserved.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND,
 * either express or implied. See the License for the specific
 * language governing permissions and limitations under the License.
 *
 ****************************************************************************/

/**
 * @defgroup ARAUI AraUI Framework
 * @{
 */
/**
 * @defgroup CORE Core
 * @ingroup ARAUI
 * @{
 */

#ifndef __UI_CORE_H__
#define __UI_CORE_H__

#include <araui/ui_commons.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Start the UI Core Service.
 *
 * @return On success, UI_OK is returned. On failure, the defined error type is returned.
 */
ui_error_t ui_start(void);

/**
 * @brief Stop the UI Core Service.
 *
 * @return On success, UI_OK is returned. On failure, the defined error type is returned.
 */
ui_error_t ui_stop(void);

#ifdef __cplusplus
}
#endif

#endif

/** @} */ // end of CORE group

/** @} */ // end of ARAUI group
