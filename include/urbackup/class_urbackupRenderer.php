<?php

class urbackupDataRenderer {

private $urbackupDataDictionnary=
    array(  "software" => array(
                "os_simple"              => "string",
                "os_version_string"      => "string",
                "client_version_string"  => "string",
                ),
            "activity" => array(
                "delete_pending"         => "string",
                "processes"              => "progressbar",
                "status"                 => "string",
                ),
            "status" => array(
                "name"                  => "string",
                "online"                => "bool",
                "lastseen"              => "uptime",
                "id"                    => "int",
                "ip"                    => "string",
                "groupname"             => "string",
                "client_version_string" => "string",
                "delete_pending"        => "string",
                "file_ok"               => "string",
                "os_simple"             => "string",
                "os_version_string"     => "string",
                "file_ok"                => "bool",
                "image_ok"               => "bool",
                "last_filebackup_issues" => "int",
                "lastbackup"             => "uptime",
                "lastbackup_image"       => "uptime",
                ),
            "backup_resume" => array(
                "file_ok"                => "bool",
                "image_ok"               => "bool",
                "last_filebackup_issues" => "int",
                "lastbackup"             => "datetime",
                "lastbackup_image"       => "datetime",
                ),
            "backups" => array(
                "archived"              => "bool",
                "backuptime"            => "datetime",
                "disable_delete"        => "bool",
                "id"                    => "string",
                "incremental"           => "string",
                "size_bytes"            => "bytes",
                ),
            "backup_images" => array(
                "backuptime"            => "datetime",
                "id"                    => "string",
                "incremental"           => "string",
                "size_bytes"            => "bytes",
                "letter"                => "string",
                ),
            "amount" => array(
                "files"                 => "string",
                "images"                => "string",
                "used"                  => "string"
                )
            );

/*private $urbackupType=array( '1' => _("Incremental file backup"),
             '2' => _("Full file backup "),
             '3' => _("Incremental image backup"),
             '4' => _("Full image backup"));
*/
    function getOutputType($info){
        return($this->urbackupDataDictionnary[$info]);
    }

    function renderProgressBar($value){
      $gauge="[#######[66%]#######.......]";
      return($gauge);
    }


    function renderUptime($uptime){
      // uptime is in * 100 second
        $sec=$uptime / 100;
        $years=intval($sec/(3600*24*365));
        $days = intval(($sec - ($years* 3600*24*365))/(3600*24));
        $hours = intval(($sec - ($years*3600*24*365 + $days*24*3600 ))/3600);
        $minutes = intval(($sec - ($years*3600*24*365 + $days *24*3600 + $hours * 3600 ))/60);

        return( $years." "._("years")." ".$days." "._("days")." ".$hours." "._("hours")." ".$minutes." "._("minutes"));
    }


    function renderBytes($size){
         $size = $value / 1024 / 1024;
        return($size." Mo");
    }

    function renderDateTime($value){
        return(strftime("%e/%m/%Y - %T",$value));
    }

    function getRenderValue($type='', $value = ''){
        if ( $value ){
            if (is_string($type)){
                switch ($type) {
                    case "bool":
                        if ($value == "1"){
                            return("True");
                        }else{
                            return("False");
                        }
                        break;
                    case "datetime":
                        return($this->renderDateTime($value));
                        break;
                    case "string":
                        return($value);
                        break;
                    case "int":
                        return($value);
                        break;
                    case "progressbar":
                        return($this->renderProgressBar($value));
                        break;
                    case "uptime":
                        return($this->renderUptime($value));
                        break;
                    case "bytes":
                        return($this->renderBytes($value));
                        break;

                    default:
                        return ($value);
                }
            }else{
                // rendere type is an array
                $arrayValue=array();
                if (is_object($value)){
                    // if value is an objet
                    foreach ($type['fields'] as $key){
                        $arrayValue[] = $value->$key;
                    }
                return(vsprintf($type['format'],$arrayValue));
                }
            }
        }else{
            return("no value(s) found");
        }
    }




}
  ?>
