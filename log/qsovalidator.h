/*
 * This file is part of QHamLog
 * Copyright (C) 2013 Alex Gladd
 *
 * QHamLog is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * QHamLog is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with QHamLog.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef QSOVALIDATOR_H
#define QSOVALIDATOR_H

#include "qsolog.h"
#include <QString>

namespace log
{

class QsoValidator
{
public:
    /**
     * @brief Validate the given QSO record
     * @param qso the QSO record to validate
     * @return true if the give QSO record is valid
     */
    static bool validateQso(const Qso &qso);

    /**
     * @brief Get the last error message (e.g., after a validator failure)
     * @return
     */
    static QString getLastError();

    /**
     * @brief Destructor
     */
    virtual ~QsoValidator();

protected:
    /**
     * @brief Constructor (should never need to instantiate)
     */
    QsoValidator();

    /**
     * @brief Internal setter for the last error message
     * @param errMsg the message to set
     */
    static void setLastError(const QString &errMsg);

    /* static storage for the last error msg */
    static QString lastError;
};

} // namespace log

#endif // QSOVALIDATOR_H
