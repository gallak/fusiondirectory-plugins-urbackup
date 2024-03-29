<?php

/*
  This code is an addon to FusionDirectory (https://www.fusiondirectory.org/)
  Copyright (C) 2021 Antoine Gallavardin

  This program is free software; you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation; either version 2 of the License, or
  (at your option) any later version.

  This program is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License for more details.

  You should have received a copy of the GNU General Public License
  along with this program; if not, write to the Free Software
  Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA 02110-1301, USA.
*/

class serviceUrbackup extends simpleService
{
  static function plInfo (): array
  {
    return [
      'plShortName'   => _('Urbackup Service'),
      'plDescription' => _('Urbackup Service').' ('._('Services').')',
      'plIcon'        => 'geticon.php?context=applications&icon=urbackup&size=16',
      'plObjectClass' => ['fdUrbackupService'],

      'plProvidedAcls'  => parent::generatePlProvidedAcls(static::getAttributesInfo())
    ];
  }

  /*!
   *  \brief The main function : information about attributes
   */
  static function getAttributesInfo (): array
  {
    return  [
      'main' => [
        'name'  => _('Urbackup Service'),
        'attrs' => [
          new StringAttribute(
            _('URL'), _('URL to access the urbackup server'),
            'fdUrbackupServiceURL', TRUE
          ),
          new StringAttribute(
            _('User'), _('User to access urbackup server Rest API.'),
            'fdUrbackupServiceUser', FALSE
          ),
          new PasswordAttribute(
            _('Password'), _('Password to access urbackup server Rest API.'),
            'fdUrbackupServicePassword', FALSE
          )
        ]
      ],
    ];
  }
}
