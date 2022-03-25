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

// TODO
// traduction des champs des tableaux urbavkaup
// respecter les norme de codage de function . variable / champ LDAP
// ajouter un rendere pour la progress bar
// message d'erreur en cas de non existence de backup ou client


require_once("urbackup/class_urbackup.php");
require_once("urbackup/class_urbackupRenderer.php");

  function getAllUrbackupServers()
  {
      global $config;
      $ldap = $config->get_ldap_link();
      $allUrbackServers=array();
      $ldap->search('(objectClass=fdUrbackupService)', ['dn','cn','description']);
      while ($uServer = $ldap->fetch()) {
      $allUrbackServers[$uServer['dn']]= array(  'urbackupCn' => $uServer['cn'][0],
                                                    'urbackupDescription' => $uServer['description'][0]);
      }
      return($allUrbackServers);
  }



class UrbackupServerSelectAttribute extends SelectAttribute
{
  function setParent (&$plugin)
  {
    parent::setParent($plugin);
    if (is_object($this->plugin)) {
      $this->updateChoices();
    }
  }

  function updateChoices ()
  {
    global $config;
    $serversName    = [];
    $serversDescription = [];
    foreach (getAllUrbackupServers() as $name => $content){
      $serversDescription[] = $content['urbackupCn']." ".$content['urbackupDescription'];
      $serversDn[] = $name;
      $this->setChoices($serversDn, $serversDescription);
    }
  }
}





class urbackupClient extends simplePlugin
{

  private $urbackupHandler;
  protected $displayHeader  = TRUE;
  protected $loaded = FALSE;

  static function plInfo (): array
  {
    return [
      'plShortName'   => _('Urbackup'),
      'plDescription' => _('Urbackup Status'),
      'plPriority'    => 30,
      'plObjectType'  => ['workstation'],
      'plObjectClass'     => ['fdUrbackupClient'],
      'plProvidedAcls'  => parent::generatePlProvidedAcls(static::getAttributesInfo())
    ];
  }

  static function getAttributesInfo (): array
  {
    return [
      'parametrage' => [
        'name'  => _('Parameter'),
        'class' => ['fullwidth'],
        'attrs' => [
              new StringAttribute(
            _('Client Alias'), _('Alias of client host'),
            'fdUrbackupAlias', _('Launch'),FALSE,''),
                    // choix du server
            new UrbackupServerSelectAttribute (
            _('Urbackup Server'),
            _('Name of server where the client is connected to'),
            'fdUrbackupServer',
          ),
          ]
        ],

      'hostdetails' => [
        'name'  => _('Host Details'),
        'class' => ['fullwidth'],
        'attrs' => [
          new ButtonAttribute(
            _('Refresh Informations'), _('Update informations'),
            'fdClientInfos', _('Update'),
            'submitGetDetails'
          ),
          new FakeAttribute('details_client'),
            ],
          'template' => get_template_path('urbackup_client_details.tpl', TRUE, dirname(__FILE__)),
          ],

      'actions' => [
        'name'  => _('Actions'),
        'class' => ['fullwidth'],
        'attrs' => [




///  DEBUT BOUTON BACKUP
          new ButtonAttribute(
            _('Full Image Backup'), _('Launch Full Image Backup'),
            'fdBackupFullImage', _('Launch'),
            'submitBackupFullImage'
          ),
          new ButtonAttribute(
            _('Incremental Image Backup'), _('Launch Incremental Image Backup'),
            'fdBackupIncImage', _('Launch'),
            'submitBackupIncImage'
          ),
          new ButtonAttribute(
            _('Full File Backup'), _('Launch Full File Backup'),
            'fdBackupFullFile', _('Launch'),
            'submitBackupFullFile'
          ),
          new ButtonAttribute(
            _('Incremental File Backup Backup'), _('Launch Full Image'),
            'fdBackupIncFile', _('Launch'),
            'submitBackupIncFile'
          ),
///  FIN BOUTON BACKUP
          new ButtonAttribute(
            _('Refresh Informations'), _('Update informations'),
            'fdClientInfos', _('Update'),
            'submitGetDetails'
          ),


        ]
      ],
    ];
  }

public function loadUrbackupData (){
    global $config;

    $capabilities = $this->connectUrbackupServer();
    if (isset($this->attrs['fdUrbackupAlias'][0])){
        $uClient = $this->attrs['fdUrbackupAlias'][0];
    }else{
        $uClient = $this->attrs['cn'][0];
    }

    if ($capabilities->success){
        // fetch All configuration and Data
        $this->urbackupHandler->get_usage();
        $this->urbackupHandler->get_status();
        $this->urbackupHandler->set_current_client($uClient);
    }else{
        var_dump("connxion KO");
    }
}


  public function submitGetDetails (){

        $cur_status=$this->urbackupHandler->get_current_client_status();
        $this->details_client=$this->urbackupDisplayList(array('status' => array($cur_status)),array( 'status' =>
            array('client_version_string','online','os_version_string','lastseen','lastbackup','lastbackup_image')));
    }





  function __construct ($dn = NULL, $object = NULL, $parent = NULL, $mainTab = FALSE)
  {

    parent::__construct($dn, $object, $parent, $mainTab);
    $this->is_account     = $this->is_this_account($this->attrs);
    $this->ignore_account = FALSE;  // affiche l'activation ou pas du plugin
    $this->loadUrbackupData();

  }

    function connectUrbackupServer() {

      global $config;
      $ldap = $config->get_ldap_link();
      $ldap->cd($this->fdUrbackupServer);
      $ldap->search('(objectClass=*)', ['fdUrbackupServiceURL','fdUrbackupServiceUser','fdUrbackupServicePassword']);
      $attrs = $ldap->fetch();
      $uServer = $attrs['fdUrbackupServiceURL'][0];
      $uUser   = $attrs['fdUrbackupServiceUser'][0];
      $uPass   = $attrs['fdUrbackupServicePassword'][0];
      //$uClient = $this->attrs['cn'][0];
      $this->urbackupHandler = new urbackup_service($uServer,$uUser,$uPass);
      return($this->urbackupHandler->doLogin());

    }

function urbackupDisplayTable($data,$dict){

    $urbackupData = new urbackupDataRenderer();
    $arrayDiv=array();
    $smarty = get_smarty();

    foreach ($data as $item => $values){
        if ($values) {
            $div = new divSelectBox('rows'.$item);
            // set height depending values
            $div->setHeight(count(array_keys($values)) * 26 + 40);
            $headers=array();
            foreach ($dict[$item] as $field) {
                $headers[]=_($field);
            }
            $div->setHeaders($headers);

            foreach ($values as &$record) {
                $fields=array();
                var_dump($record);
                foreach ($dict[$item] as $field){
                    $fields[]=[ 'string' => $urbackupData->getRenderValue($urbackupData->getOutputType($item)[$field] ,$record->$field)];
                }
                $div->addEntry($fields);
            }
            $arrayDiv[$item] = $div->drawList();
        }

    }
    return $arrayDiv;
}

  function urbackupDisplayList($data,$dict){
    //$id   = $category;

    $urbackupData = new urbackupDataRenderer();
    $id = array_key_first($dict);
    $arrayData = (array)$data[$id][0];
    $div  = new divSelectBox('rows'.$id);
    $smarty = get_smarty();
    // take the first element of array data
    $div->setHeight(count($dict[$id]) * 27);
    $fields=[];
    $clientInfo=$category;
      foreach ($dict[$id] as $key ){
          $fields = [
             ['string' => _($key)],
             ['string' => $urbackupData->getRenderValue($urbackupData->getOutputType($id)[$key] ,$arrayData[$key])],
            ];
    $div->addEntry($fields);

        }
   return $div->drawList();
  }




}
