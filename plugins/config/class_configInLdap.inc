<?php
/*
  This code is part of FusionDirectory (http://www.fusiondirectory.org/)
  Copyright (C) 2012-2016 FusionDirectory

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

/* Debug level is an OR combination of several values */
class DebugLevelAttribute extends SetAttribute
{
  protected function loadAttrValue ($attrs)
  {
    if (isset($attrs[$this->getLdapName()])) {
      $value = $attrs[$this->getLdapName()][0];
      $this->value = array();
      foreach ($this->attribute->getChoices() as $choice) {
        if ($value & $choice) {
          $this->value[] = $choice;
        }
      }
    } else {
      $this->resetToDefault();
    }
  }

  function computeLdapValue()
  {
    $value = 0;
    foreach ($this->value as $v) {
      $value |= $v;
    }
    return $value;
  }
}

class configInLdap extends simplePlugin
{
  var $objectclasses  = array("fusionDirectoryConf");

  static function plInfo()
  {
    return array(
      'plShortName'     => _('Configuration'),
      'plTitle'         => _('FusionDirectory configuration'),
      'plDescription'   => _('Configuration screen of FusionDirectory'),
      'plIcon'          => 'geticon.php?context=categories&icon=settings&size=48',
      'plObjectType'    => array(
        'configuration' => array(
          'name'      => _('FusionDirectory configuration'),
          'filter'    => 'objectClass=fusionDirectoryConf',
          'tabClass'  => 'tabs_configInLdap',
          'icon'      => 'geticon.php?context=categories&icon=settings&size=16',
          'mainAttr'  => FALSE,
          'ou'        => preg_replace('/^[^,]+,/', '', CONFIGRDN)
        )
      ),
      'plSection'       => array('conf' => array('name' => _('Configuration'), 'priority' => 20)),
      'plManages'       => array('configuration'),
      'plPriority'      => 0,

      'plProvidedAcls'  => parent::generatePlProvidedAcls(static::getAttributesInfo())
    );
  }

  static function getAttributesInfo ()
  {
    global $config;
    $plugins = array();
    if (session::global_is_set('plist')) {
      $plugins = array_keys(session::global_get('plist')->info);
    }
    sort($plugins);
    return array(
      'look_n_feel' => array(
        'name'  => _('Look and feel'),
        'attrs' => array(
          new SelectAttribute (
            _('Language'), _('Language of the application. If \'automatic\' or not available, the one asked by the browser will be used. This setting can be overriden per user.'),
            'fdLanguage', FALSE,
            array('')
          ),
          new SelectAttribute (
            _('Theme'), _('Theme to be used'),
            'fdTheme', TRUE,
            static::get_themes(),
            'breezy'
          ),
          new SelectAttribute (
            _('Timezone'), _('Timezone to be used'),
            'fdTimezone', TRUE,
            array('America/New_York')
          ),
          new HiddenAttribute ('fusionConfigMd5'),
        )
      ),
      'core_settings' => array(
        'name'  => _('Core settings'),
        'attrs' => array(
          new IntAttribute (
            _('LDAP size limit'), _('Defines the number of entries to get from LDAP by default.'),
            'fdLdapSizeLimit', FALSE,
            0 /*min*/, FALSE /*no max*/, 200
          ),
          new SelectAttribute (
            _('Edit locking'),
            _('Check if a entry currently being edited has been modified outside of FusionDirectory in the meantime.'),
            'fdModificationDetectionAttribute', FALSE,
            array('', 'entryCSN', 'contextCSN'), 'entryCSN'
          ),
          new BooleanAttribute (
            _('Enable logging'),
            _('Event logging on FusionDirectory side.'),
            'fdLogging', FALSE,
            TRUE
          ),
          new BooleanAttribute (
            _('Schema validation'),
            _('Enables schema checking during login.'),
            'fdSchemaCheck', FALSE,
            TRUE
          ),
          new BooleanAttribute (
            _('Enable snapshots'), _('This enables you to save certain states of entries and restore them later on.'),
            'fdEnableSnapshots', FALSE,
            TRUE
          ),
          new StringAttribute (
            _('Snapshot base'), _('The base where snapshots should be stored inside of the LDAP.'),
            'fdSnapshotBase', FALSE,
            'ou=snapshots,'.$config->current['BASE']
          ),
        )
      ),
      'password' => array(
        'name'  => _('Password settings'),
        'attrs' => array(
          new SelectAttribute (
            _('Password default hash'), _('Default hash to be used'),
            'fdPasswordDefaultHash', TRUE,
            array('ssha')
          ),
          new BooleanAttribute (
            _('Force default hash'), _('Force the use of the default password hash'),
            'fdForcePasswordDefaultHash'
          ),
          new IntAttribute (
            _('Password minimum length'), _('Minimum length of user passwords'),
            'fdPasswordMinLength', FALSE,
            0 /*min*/, FALSE /*no max*/
          ),
          new IntAttribute (
            _('Password minimum differs'), _('Minimum number of different characters from last password'),
            'fdPasswordMinDiffer', FALSE,
            0 /*min*/, FALSE /*no max*/
          ),
          new BooleanAttribute (
            _('Use account expiration'),
            _('Enables shadow attribute tests during the login to FusionDirectory and forces password renewal or account locking'),
            'fdHandleExpiredAccounts'
          ),
          new StringAttribute (
            _('SASL Realm'), _('SASL Realm'),
            'fdSaslRealm'
          ),
          new StringAttribute (
            _('SASL Exop'), _('Attribute to be stored in the userPassword attribute'),
            'fdSaslExop'
          ),
        )
      ),
      'login' => array(
        'name'  => _('Login and session'),
        'attrs' => array(
          new SelectAttribute (
            _('Login attribute'),
            _('Which LDAP attribute should be used as the login name during login.'),
            'fdLoginAttribute', TRUE,
            array('uid', 'mail', 'uid,mail'), 'uid',
            array('uid', 'mail', 'both')
          ),
          new BooleanAttribute (
            _('Enforce encrypted connections'),
            _('Enables PHP security checks to force encrypted access (https) to the web interface.'),
            'fdForceSSL'
          ),
          new BooleanAttribute (
            _('Warn if session is not encrypted'),
            _('will display a warning to the user when http is used instead of https.'),
            'fdWarnSSL', FALSE,
            TRUE
          ),
          new IntAttribute (
            _('Session lifetime'), _('Defines when a session will expire in seconds (0 to disable).'),
            'fdSessionLifeTime', TRUE,
            0 /*min*/, FALSE /*no max*/, 1800
          ),
          new BooleanAttribute (
            _('HTTP Basic authentication'), _('Use HTTP Basic authentication protocol instead of the login form.'),
            'fdHttpAuthActivated', FALSE,
            FALSE
          ),
          new BooleanAttribute (
            _('HTTP Header authentication'), _('Use HTTP Header authentication instead of the login form.'),
            'fdHttpHeaderAuthActivated', FALSE,
            FALSE
          ),
          new StringAttribute (
            _('Header name'), _('Name of the header containing user identifier.'),
            'fdHttpHeaderAuthHeaderName', FALSE,
            'AUTH_USER'
          ),
        )
      ),
      'ssl' => array(
        'name'  => _('SSL'),
        'attrs' => array(
          new TrimmedStringAttribute (
            _('Key path'), _('Path to FusionDirectory private key. Unused for now.'),
            'fdSslKeyPath', FALSE,
            '/etc/ssl/private/fd.key'
          ),
          new TrimmedStringAttribute (
            _('Certificate path'), _('Path to FusionDirectory certificate. Unused for now.'),
            'fdSslCertPath', FALSE,
            '/etc/ssl/certs/fd.cert'
          ),
          new TrimmedStringAttribute (
            _('CA certificate path'), _('Path to the CA certificate. Used for validating Argonaut Server host.'),
            'fdSslCaCertPath', FALSE,
            '/etc/ssl/certs/ca.cert'
          ),
        )
      ),
      'cas' => array(
        'name'  => _('CAS'),
        'attrs' => array(
          new BooleanAttribute (
            _('Enable CAS'), _('CAS login will be used instead of LDAP bind'),
            'fdCasActivated', FALSE,
            FALSE
          ),
          new TrimmedStringAttribute (
            _('CA certificate path'), _('Path to the CA certificate of the CAS server'),
            'fdCasServerCaCertPath', FALSE,
            '/etc/ssl/certs/ca.cert'
          ),
          new StringAttribute (
            _('Host'), _('Host of the CAS server'),
            'fdCasHost', FALSE,
            'localhost'
          ),
          new IntAttribute (
            _('Port'), _('Port the CAS server is listening on'),
            'fdCasPort', FALSE,
            0 /*min*/, FALSE /*no max*/, 443
          ),
          new StringAttribute (
            _('CAS context'), _('CAS context to be used'),
            'fdCasContext', FALSE,
            '/cas'
          ),
        )
      ),
      'people_and_group' => array(
        'name'  => _('People and group storage'),
        'class' => array('critical'),
        'attrs' => array(
          new SelectAttribute (
            _('People DN attribute'), _('Attribute to use at the beginning of users dn'),
            'fdAccountPrimaryAttribute', TRUE,
            array('uid', 'cn')
          ),
          new StringAttribute (
            _('CN pattern'), _('The pattern to use to build the common name field'),
            'fdCnPattern', TRUE,
            '%givenName% %sn%'
          ),
          new BooleanAttribute (
            _('Strict naming policy'),
            _('Enables strict checking of user and group names'),
            'fdStrictNamingRules', FALSE,
            TRUE
          ),
          new StringAttribute (
            _('Users RDN'), _('The branch where users are stored.'),
            'fdUserRDN', TRUE,
            'ou=people'
          ),
          new StringAttribute (
            _('ACL role RDN'), _('The branch where ACL roles are stored.'),
            'fdAclRoleRDN', TRUE,
            'ou=aclroles'
          ),
          new BooleanAttribute (
            _('Restrict role members'), _('When enabled only users from the same branch or members of groups from the same branch can be added to a role.'),
            'fdRestrictRoleMembers'
          ),
          new BooleanAttribute (
            _('Separate address fields'), _('Expose street, postOfficeBox and postalCode fields instead of postalAddress.'),
            'fdSplitPostalAddress'
          ),
          new PostalAddressAttribute (
            _('Postal address pattern'), _('When using separate address fields, you can use a pattern to fill postalAddress field.'),
            'fdPostalAddressPattern'
          ),
        )
      ),
      'debug' => array(
        'name'  => _('Debugging'),
        'attrs' => array(
          new BooleanAttribute (
            _('Display errors'),
            _('Shows PHP errors in the upper part of the screen. This should be disabled in productive deployments, because there might be some passwords in it.'),
            'fdDisplayErrors'
          ),
          new IntAttribute (
            _('Maximum LDAP query time'), _('Stop LDAP actions if there is no answer within the specified number of seconds.'),
            'fdLdapMaxQueryTime', FALSE,
            0 /*min*/, FALSE /*no max*/
          ),
          new BooleanAttribute (
            _('Log LDAP statistics'),
            _('Track LDAP timing statistics to the syslog. This may help to find indexing problems or bad search filters.'),
            'fdLdapStats'
          ),
          new DebugLevelAttribute (
            new SelectAttribute (
              _('Debug level'),
              _('Display certain information on each page load.'),
              'fdDebugLevel', FALSE,
              array(DEBUG_TRACE,  DEBUG_LDAP, DEBUG_DB,   DEBUG_SHELL,  DEBUG_POST,
                    DEBUG_SESSION,  DEBUG_ACL,  DEBUG_SI, DEBUG_MAIL),
                    DEBUG_TRACE,
              array('Trace',      'LDAP',     'Database', 'Shell',      'POST',
                    'SESSION',      'ACL',      'SI',     'Mail')
            )
          ),
        )
      ),
      'miscellaneous' => array(
        'name'  => _('Miscellaneous'),
        'attrs' => array(
          new BooleanAttribute (
            _('Display summary in listings'),
            _('Determines whether a status bar will be shown on the bottom of lists, displaying a short summary of type and number of elements in the list.'),
            'fdListSummary', FALSE,
            TRUE
          ),
          new BooleanAttribute (
            _('Show ACL tab on all objects'),
            _('For very specific ACL rights setting where you might need to give right on a single object.'),
            'fdAclTabOnObjects'
          ),
          new SetAttribute(
            new StringAttribute (
              _('Available department categories'), _('Available categories in the departments dropdown'),
              'fdDepartmentCategories', FALSE
            ),
            array()
          ),
          new OrderedArrayAttribute(
            new PipeSeparatedCompositeAttribute(
              _('Use this to hide some menu entry to specific groups of users'),
              'fdPluginsMenuBlacklist',
              array(
                new SelectAttribute(
                  '', _('Group or role'),
                  'blacklistGroup', TRUE,
                  array()
                ),
                new SelectAttribute(
                  '', _('Plugin to blacklist'),
                  'blacklistPlugin', TRUE,
                  array()
                ),
              ),
              '',
              _('Plugin menu blacklist')
            ),
            // no order
            FALSE,
            array()
          )
        )
      ),
      'hooks' => array(
        'name'  => _('Hooks'),
        'class' => array('fullwidth'),
        'attrs' => array(
          new OrderedArrayAttribute (
            new CompositeAttribute (
              _('Hooks that are called when specific actions happens'),
              'fdTabHook',
              array(
                new SelectAttribute(
                  _('Tab'), _('The tab that this hook concerns'),
                  'hookTab', TRUE,
                  $plugins
                ),
                new SelectAttribute(
                  _('Mode'), _('When to call this command'),
                  'hookMode', TRUE,
                  array('postcreate', 'postremove', 'postmodify',
                        'precreate', 'preremove', 'premodify', 'check',
                        'prelock', 'postlock', 'preunlock', 'postunlock')
                ),
                new TextAreaAttribute(
                  _('Command'), _('The command that will be called'),
                  'hookCmd', TRUE
                )
              ),
              '/^([^\\|]+)\\|([^\\|]+)\\|(.*)$/',
              '%s|%s|%s',
              '', // acl
              _('Hooks')
            ),
            FALSE, // non-ordered
            array(),
            TRUE // edition
          ),
          new BooleanAttribute (
            _('Display hook output'),
            _('When enabled successful hook execution output is displayed to the user using a dialog.'),
            'fdDisplayHookOutput'
          ),
        )
      ),
    );
  }

  function __construct ($dn = NULL, $object = NULL, $parent = NULL, $mainTab = FALSE, $attributesInfo = NULL)
  {
    global $config;
    $attributesInfo = static::getAttributesInfo();
    /* Languages */
    $languages = get_languages(TRUE);
    $languages = array_merge(array("" => _("Automatic")), $languages);
    $attributesInfo['look_n_feel']['attrs'][0]->setChoices(array_keys($languages), array_values($languages));
    /* Timezones */
    $attributesInfo['look_n_feel']['attrs'][2]->setChoices(timezone::_get_tz_zones());
    /* Password methods */
    $methods = passwordMethod::get_available_methods();
    $methods = $methods['name'];
    if (!in_array('sasl', $methods)) {
      $methods[] = 'sasl';
    }
    $attributesInfo['password']['attrs'][0]->setChoices($methods);

    $groupsAndRoles = array_merge(
      array_map(
        function ($group)
        {
          return sprintf(_('Group %s'), $group);
        },
        objects::ls('ogroup')
      ),
      array_map(
        function ($role)
        {
          return sprintf(_('Role %s'), $role);
        },
        objects::ls('role')
      )
    );
    $attributesInfo['miscellaneous']['attrs'][3]->attribute->attributes[0]->setChoices(
      array_keys($groupsAndRoles),
      array_values($groupsAndRoles)
    );
    $menuPlugins = array();
    $plist = session::global_get('plist');
    foreach ($config->data['SECTIONS'] as $section => $section_infos) {
      foreach ($config->data['MENU'][$section] as $info) {
        if (isset($info['CLASS'])) {
          list ($plHeadline, , , ) = $plist->get_infos($info['CLASS']);
          $menuPlugins[$info['CLASS']] = $plHeadline;
        }
      }
    }
    asort($menuPlugins);
    $attributesInfo['miscellaneous']['attrs'][3]->attribute->attributes[1]->setChoices(
      array_keys($menuPlugins),
      array_values($menuPlugins)
    );

    try {
      parent::__construct($dn, $object, $parent, $mainTab, $attributesInfo);
    } catch (NonExistingLdapNodeException $e) {
      parent::__construct('new', $object, $parent, $mainTab, $attributesInfo);
      $this->dn = $dn;
    }

    $this->fusionConfigMd5 = md5_file(CACHE_DIR."/".CLASS_CACHE);

    $this->attributesAccess['fdHttpAuthActivated']->setManagedAttributes(
      array(
        'erase' => array (
          TRUE => array (
            'fdCasActivated',
          )
        )
      )
    );
    $this->attributesAccess['fdEnableSnapshots']->setManagedAttributes(
      array(
        'disable' => array (
          FALSE => array (
            'fdSnapshotBase',
          )
        )
      )
    );

    $this->attributesAccess['fdTabHook']->setLinearRendering(FALSE);

    $this->attributesAccess['fdForceSSL']->setManagedAttributes(
      array(
        'disable' => array (
          TRUE => array (
            'fdWarnSSL',
          )
        )
      )
    );
    $this->attributesAccess['fdCasActivated']->setManagedAttributes(
      array(
        'disable' => array (
          FALSE => array (
            'fdCasServerCaCertPath',
            'fdCasHost',
            'fdCasPort',
            'fdCasContext',
          )
        )
      )
    );
    $this->attributesAccess['fdHttpHeaderAuthActivated']->setManagedAttributes(
      array(
        'disable' => array (
          FALSE => array (
            'fdHttpHeaderAuthHeaderName',
          )
        )
      )
    );
    $this->attributesAccess['fdSplitPostalAddress']->setManagedAttributes(
      array(
        'disable' => array (
          FALSE => array (
            'fdPostalAddressPattern',
          )
        )
      )
    );
  }

  function compute_dn()
  {
    return $this->dn;
  }

  function check()
  {
    $messages = parent::check();
    if (($this->fdPasswordDefaultHash == 'sasl') && ($this->fdSaslRealm == '') && ($this->fdSaslExop == '')) {
      $messages[] = _('You need to fill saslRealm or saslExop in the configuration screen in order to use SASL');
    }
    return $messages;
  }

  static function get_themes()
  {
    $themesdir  = '../ihtml/themes/';
    $themes     = array_keys(session::global_get(IconTheme::$session_var));
    if ($dir = opendir("$themesdir")) {
      while (($file = readdir($dir)) !== FALSE) {
        if (is_dir("$themesdir/$file") && !preg_match("/^\./", $file)) {
          $themes[] = $file;
        }
      }
    }
    return array_unique($themes);
  }
}
?>
