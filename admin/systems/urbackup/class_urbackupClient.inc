<?php
/*
  This code is part of FusionDirectory (http://www.fusiondirectory.org/)
  Copyright (C) 2016-2017  FusionDirectory

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



class SubmittingUrbackupServerAttribute extends SelectAttribute
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
    $ldap = $config->get_ldap_link();
    $ldap->cd($config->current['BASE']);
    $ldap->search('(objectClass=urbackupServer)', ['cn','description']);
    $serversdn    = [];
    $serverslabel = [];
    while ($attrs = $ldap->fetch()) {
      $serversdn[]    = $attrs['dn'];
      $serverslabel[] = $attrs['cn'][0] ." - ".$attrs['description'][0];
    }
    $this->setChoices($serversdn, $serverslabel);
  }
}







class urbackupClient extends simplePlugin
{

  public $urbackupHandler;

  protected $displayHeader  = TRUE;
  protected $loaded = FALSE;

  static function plInfo (): array
  {
    return [
      'plShortName'   => _('Urbackup'),
      'plDescription' => _('Urbackup Status'),
      'plPriority'    => 30,
      'plObjectType'  => ['workstation'],
      'plObjectClass'     => ['urbackupClient'],
      'plProvidedAcls'  => parent::generatePlProvidedAcls(static::getAttributesInfo())
    ];
  }





  public function refresh ()
  {
    global $config;

    $capabilities = $this->connectUrbackupServer();
    $uClient = $this->attrs['cn'][0];

 

    $ldap = $config->get_ldap_link();
      $ldap->cd($this->urbackupAffectationServer);
      $ldap->search('(objectClass=*)', ['urbackupServerURL','urbackupServerUser','urbackupServerPassword']);
      $attrs = $ldap->fetch();

      $uServer = $attrs['urbackupServerURL'][0];
      $uUser   = $attrs['urbackupServerUser'][0];
      $uPass   = $attrs['urbackupServerPassword'][0];
      $uClient = $this->attrs['cn'][0];
     
      $this->urbackupHandler = new urbackup_server($uServer,$uUser,$uPass);
      $capabilities = $this->urbackupHandler->login();

      //var_dump($capabilities);


      if ($capabilities['resultat'] == true){
	// fetch client informations
        $host_detail = $this->urbackupHandler->get_client_status($uClient);
        if ($host_detail['resultat'] == true){
          $this->details_client_software=$this->displayDetails($host_detail['obj'],"software");
          $this->details_client_activity=$this->displayDetails($host_detail['obj'],"activity");
          $this->details_client_status=$this->displayDetails($host_detail['obj'],"status");
          $this->details_client_backup_resume=$this->displayDetails($host_detail['obj'],"backup_resume");

	  $hostid = $host_detail['obj']->id;
	}
        $backup_img_details = $this->urbackupHandler->get_client_backups($hostid);
        if ($backup_img_details['resultat'] == true){
		$this->details_image_backups=$this->backup_details($backup_img_details['obj'],'backup_images');
		$this->details_file_backups =$this->backup_details($backup_img_details['obj'],'backups');
	}

     }else{
	     $this->details_file_backups = $this->backup_details("No files backup",'backups');
	     $this->details_image_backups = $this->backup_details("No Images backup", "backup_images");
	     $this->details_client = $this->displayDetails("no client");
     }


  }




  private function getHTMLArray  ($obj): string
  {
    $html = "";
    foreach(json_decode(json_encode($obj), true) as $k => $v){
      $html = $html.$k." = ".$v."<br>";
    }
    return($html);
  }



  private function dataRenderer ($type, $value){

   $status=[ '1' => _("Incremental file backup"),
             '2' => _("Full file backup "),
             '3' => _("Incremental image backup"),
             '4' => _("Full image backup")];


    switch ($type) {
      case "status":
        if ($value){
		return($status[$value]." ". _(" in progress"));
	}else{
		return(_("no jobs"));
	}
        break;

      case "bool":
        if ($value){
          return(_("True"));
        }else{
          return(_("False"));
        }
        break;
      case "progressbar":
        $progress=(array) $value[0];
        if ($progress) {
          $action=$progress['action'];
          if ( $progress['pcdone'] == "-1" ){ 
            $msg = _("preparation");
          }else{
            $msg = _("Progession")." ".$progress['pcdone']." %";
          }
          return($status[$action]. " : ".$msg);
        }else{
          return(_("no jobs"));
        }
        break;

      case "datetime":
        return(strftime("%e/%m/%Y - %T",$value));
        break;
      case "string":
        return($value);
        break;
      case "int":
        return($value);
      case "bytes" :
        $size = $value / 1024 / 1024 / 1024;
        return($size." Go");
      default:
        return ($value);
    } 
  }

  public function displayDetails ($jsonText = '',$category="software"){
    $arr = (array) $jsonText;

    $typeDetails = array("software" => array(
                                          "os_simple"              => array( "label" => _("Os Simple"), "type" => "string"),
                                          "os_version_string"      => array( "label" => _("Os Detailed"), "type" => "string"),
                                          "client_version_string"  => array( "label" => _("Urbackup version"), "type" => "string"),
                                          ),
                         "activity" => array(
                                          "delete_pending"         => array( "label" => _("Pending Deletion"), "type" => "string"),
                                          "processes"              => array( "label" => _("Processes"), "type" => "progressbar"),
                                          "status"                 => array( "label" => _("Status"), "type" => "string"),
                                          ),
                         "status" => array(
                                          "name"                   => array( "label" => _("Client Name"), "type" => "string"),
                                          "online"                 => array( "label" => _("On Line"), "type" => "bool"),
                                          "lastseen"               => array( "label" => _("Last seen"), "type" => "datetime"),
                                          "id"                     => array( "label" => _("numeric Id"), "type" => "int"),
                                          "ip"                     => array( "label" => _("Ip Address"), "type" => "string"),
                                          "groupname"              => array( "label" => _("Groups"), "type" => "string"),
                                          ),
                         "backup_resume" => array(
                                          "file_ok"                => array( "label" => _("Last File Backup state"), "type" => "bool"),
                                          "image_ok"               => array( "label" => _("Last Image Backup state"), "type" => "bool"),
                                          "last_filebackup_issues" => array( "label" => _("Last filebackup issues"), "type" => "int"),
                                          "lastbackup"             => array( "label" => _("Last file backup date"), "type" => "datetime"),
                                          "lastbackup_image"       => array( "label" => _("Last image backup date"), "type" => "datetime"),
                                         )
                         );



    $id   = $category;
    $div  = new divSelectBox('rows'.$id);
    $smarty = get_smarty();
    $div->setHeight(count(array_keys($typeDetails[$category])) * 30);

    $fields=[];
    $clientInfo=$category;
      foreach ($typeDetails[$clientInfo] as $infoKey => $infoType){
          $fields = [ 
             ['string' => $infoType['label']],
             ['string' => $this->dataRenderer($infoType['type'],$arr[$infoKey])],
            ];
	$div->addEntry($fields);

        }
   return $div->drawList();
  }




  public function backup_details ($jsonText = '', $typeBackup = 'backups'){
    $arr = json_decode($jsonText, true);
    $backupInfos = array(
                        "backup_images" => array(
                                            "id"           => array("label" => _("Numeric Id"), "type" => "int"),
                                            "archived"     => array("label" => _("Archived"), "type" =>"bool"),
                                            "backuptime"   => array("label" => _("Backup time"), "type" =>"datetime"),
                                            "incremental"  => array("label" => _("Incremental ?"), "type" =>"bool"),
                                            "letter"       => array("label" => _("Drive"), "type" =>"string"),
                                            "size_bytes"   => array("label" => _("Size"), "type" =>"bytes"),
                                       ),
                         "backups" => array(
                                       "id"                => array("label" => _("Numeric Id"), "type" =>"int"),
                                       "archived"          => array("label" => _("Archived"), "type" =>"bool"),
                                       "backuptime"        => array("label" => _("Backup time"), "type" =>"datetime"),
                                       "disable_delete"    => array("label" => _("Deletion disabled"), "type" =>"bool"),
                                       "incremental"       => array("label" => _("Incremental ?"), "type" =>"bool"),
                                       "size_bytes"        => array("label" => _("Size"), "type" =>"bytes")
                                       ));

    $id   = $typeBackup;
    $div  = new divSelectBox('rows'.$id);
    $smarty = get_smarty();
    $div->setHeight(180);
    foreach (array_keys($backupInfos[$typeBackup]) as $label){
      $labels[]=$label['label'];
    }
    $div->setHeaders(array_keys($backupInfos[$typeBackup]));

    for ($x = 0; $x < count($arr[$typeBackup]); $x++) {
        $fields=[];
        foreach ($backupInfos[$typeBackup] as $infoKey => $infoType){
	  $fields[] = [ 'string' => $this->dataRenderer($infoType['type'],$arr[$typeBackup][$x][$infoKey])];
        }
     $div->addEntry($fields);
   }
   return $div->drawList();
}



  static function getAttributesInfo (): array
  {
    return [
      'actions' => [
        'name'  => _('Actions'),
      //  'class' => ['fullwidth'],
        'attrs' => [

          new SubmittingUrbackupServerAttribute(
            _('Urbackup server'), _('Urbackup used for backup'),
            'urbackupAffectationServer', TRUE
          ),
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
          new ButtonAttribute(
            _('Refresh Informations'), _('Update informations'),
            'fdClientInfos', _('Update'),
            'refresh'
          ),



        ]
      ],

      'clientSoftware' => [
        'name'  => _('Client Software informations'),
        'attrs' => [
           new FakeAttribute('details_client_software'),
        ],
        'template' => get_template_path('urbackup_client.tpl', TRUE, dirname(__FILE__)),
      ],
      'clientActivity' => [
        'name'  => _('Client Activity'),
        'attrs' => [
           new FakeAttribute('details_client_activity'),
        ],
        'template' => get_template_path('urbackup_client.tpl', TRUE, dirname(__FILE__)),
      ],
      'clientStatus' => [
        'name'  => _('Client Status'),
        'attrs' => [
           new FakeAttribute('details_client_status'),
        ],
        'template' => get_template_path('urbackup_client.tpl', TRUE, dirname(__FILE__)),
      ],
      'clientBackupResume' => [
        'name'  => _('Client Backup resume'),
        'attrs' => [
           new FakeAttribute('details_client_backup_resume'),
        ],
        'template' => get_template_path('urbackup_client.tpl', TRUE, dirname(__FILE__)),
      ],




     'jobs' => [
        'name'  => _('Client Backup Jobs'),
        'class' => ['fullwidth'],
        'attrs' => [
          new DisplayAttribute(
            '', _('Details of Current Job'),
            'display_job_details'
           ),

        ]
      ],



      'backupImage' => [
        'name'  => _('Image Backup'),
        'class' => ['fullwidth'],    
        'attrs' => [
            new FakeAttribute('details_image_backups'),
          ],
        'template' => get_template_path('urbackup_backup.tpl', TRUE, dirname(__FILE__)),
      ],
      'backupFile' => [
        'name'  => _('File Backup'),
        'class' => ['fullwidth'],    
        'attrs' => [
            new FakeAttribute('details_file_backups')
          ],
        'template' => get_template_path('urbackup_backup.tpl', TRUE, dirname(__FILE__)),
      ],

    ];
  }


  function __construct ($dn = NULL, $object = NULL, $parent = NULL, $mainTab = FALSE)
  {
 
  parent::__construct($dn, $object, $parent, $mainTab);
  $this->refresh();

    $this->is_account     = $this->is_this_account($this->attrs);
   $this->ignore_account = FALSE;  // affiche l'activation ou pas du plugin
  }


  function prepare_save (): array
  {
    global $config;

    $errors = parent::prepare_save();
    if (!empty($errors)) {
      return $errors;
    }

//var_dump($this->urbackupType);

//    if ($this->urbackupType) {
//      try {
//        $res = $this->urbackupHandler->start_backup("client-windows",$this->urbackupType);
//        $u = new urbackup_server($uServer,$uUser,$uPass);
//        $capabilities = $u->login();
//        var_dump($this->urbackupType); 
//	var_dump($res);
//      } catch (FusionDirectoryException $e) {
//        }

//    }
return [];
}



function submitBackupFullImage () {
  $urbackupClient = $this->attrs['cn'][0];
  $urbackupType='fullImage';
  $this->connectUrbackupServer();

try {
      $res = $this->urbackupHandler->start_backup($urbackupClient,$urbackupType);
      var_dump($res);
      } catch (FusionDirectoryException $e) {
        $error = new SimplePluginError($this, htmlescape(_('Unable to launch backup')));
        $error->display();
      }
}


function submitBackupIncImage () {
  $urbackupClient = $this->attrs['cn'][0];
  $urbackupType='IncImage';
  $urBackupCnx=$this->connectUrbackupServer();

try {
        $res = $this->urbackupHandler->start_backup($urbackupClient,$urbackupType);
      var_dump($res);
      } catch (FusionDirectoryException $e) {
        $error = new SimplePluginError($this, htmlescape(_('Unable to launch backup')));
        $error->display();
      }
}

function submitBackupFullFile () {
  $urbackupClient = $this->attrs['cn'][0];
  $urbackupType='fullFile';
  $urBackupCnx=$this->connectUrbackupServer();

try {
        $res = $this->urbackupHandler->start_backup($urbackupClient,$urbackupType);
      var_dump($res);
      } catch (FusionDirectoryException $e) {
        $error = new SimplePluginError($this, htmlescape(_('Unable to launch backup')));
        $error->display();
      }
}

function submitBackupIncFile () {
  $urbackupClient = $this->attrs['cn'][0];
  $urbackupType='IncFile';
  $this->connectUrbackupServer();

try {
      $res = $this->urbackupHandler->start_backup($urbackupClient,$urbackupType);
      var_dump($res);
      } catch (FusionDirectoryException $e) {
        $error = new SimplePluginError($this, htmlescape(_('Unable to launch backup')));
        $error->display();
      }
}



function connectUrbackupServer() {

      global $config;
      $ldap = $config->get_ldap_link();
      $ldap->cd($this->urbackupAffectationServer);
      $ldap->search('(objectClass=*)', ['urbackupServerURL','urbackupServerUser','urbackupServerPassword']);
      $attrs = $ldap->fetch();

      $uServer = $attrs['urbackupServerURL'][0];
      $uUser   = $attrs['urbackupServerUser'][0];
      $uPass   = $attrs['urbackupServerPassword'][0];
      $uClient = $this->attrs['cn'][0];
      $this->urbackupHandler = new urbackup_server($uServer,$uUser,$uPass);
      return($this->urbackupHandler->login());

}


}
