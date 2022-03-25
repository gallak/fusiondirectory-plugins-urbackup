<?php

# deja declare ?
#require_once("urbackup/restclient.php");


class urbackup_service {
    private $server_url = "";
    private $username = "";
    private $password = "";
    private $headers = array("Accept: application/json","Content-Type : application/json; charset=UTF-8");
    private $session="";

    # all server properties
    private $all_status="";
    private $all_usage="";

    #
    private $current_client_id=0;
    private $current_client_name="";

    // constructeur
    public function __construct($server, $username, $password){
        $this->server_url=$server;
        $this->username=$username;
        $this->password=$password;

    }

    private function debug($level,$msg){
        switch ($level){
            case "CRITICAL":
                print("CRIT : ".$msg);
                exit(-1);
                break;
            case "WARNING":
                print("WARN : ".$msg);
                break;
            case "INFO":
                print("INFO : ".$msg);
                break;
        }
    }


    public function postInformation(string $order,array $params){
        $apiClient = new RestClient();
        // uri builder
        $curr_server_url=$this->server_url."?".http_build_query(array("a" => $order));
        // params builder
        if(strlen($this->session) > 0){
            $params=array_merge($params,array("ses" => $this->session));
        }
        $result = $apiClient->post($curr_server_url,$params,$this->headers);
        $response_json = $result->decode_response();
        return($response_json);
    }

    public function getRemoteSalt(){
        return($this->postInformation("salt",array('username' => $this->username)));
    }

    public function doLogin() {

        // get security information
        $security=$this->getRemoteSalt();
        if (empty ($security->salt)){
            $this->debug("CRITICAL","Couldn't get salt for ".$this->username."\n");
        }

        $this->session = $security->ses;
        // Password encryuption
        $password_md5_bin = md5($security->salt.$this->password,true);
        $password_md5 = utf8_decode(md5($security->salt.$this->password));
        if( $security->pbkdf2_rounds){
            $pbkdf2_rounds = $security->pbkdf2_rounds;
            if ($pbkdf2_rounds > 0) {
                $password_md5 = hash_pbkdf2('sha256', $password_md5_bin, $security->salt,$pbkdf2_rounds);
            }
        }
        $password_md5 = md5($security->rnd.$password_md5);

        // get session
        $login=$this->postInformation("login",array('username' => $this->username, 'password' => $password_md5));

        if(! isset($login) or $login->success == false){
            $this->debug("CRITICAL","Couldn't connect for ".$this->username."\n");
        }
        return($login);
    }






    # test
    public function get_current_client_settings(){
        if ($this->session){
            $params=array("sa"=>"clientsettings", "t_clientid" => $this->current_client_id);
            $settings=$this->postInformation("settings",$params);
            return($settings);
        }else{
        $this->debug("CRITICAL","User not connected");
        }
    }

    public function set_current_client($clientname){
        $this->current_client_name=$clientname;
        $this->current_client_id=$this->get_current_client_status()->id;

    }

    public function get_usage(){
    if ($this->session){
      $this->all_usage=$this->postInformation("usage",array());
    }else{
      $this->debug("CRITICAL", "User not connected");
    }
  }

    public function get_status(){
        if ($this->session){
            $this->all_status = $this->postInformation("status", array());
        }else{
            $this->debug("CRITICAL","user ".$this->username. " not logged");
        }
    }

    public function get_current_client_status(){
        $client_found=new stdClass();
            foreach($this->all_status->status as $client){
                if ( $client->name == $this->current_client_name ){
                    $client_found = $client;
                }
            }
            if (count((array)$client_found)> 0){
                return($client_found);
            }else{
                $this->debug("CRITICAL","Client ".$this->current_client_name. " not found");
            }
    }

    public function get_current_client_usage(){
        $client_found=new stdClass();
            foreach($this->all_usage->usage as $client){
                if ( $client->name == $this->current_client_name ){
                    $client_found = $client;
                }
            }
            if (count((array)$client_found)> 0){
                return($client_found);
            }else{
                $this->debug("CRITICAL","user ".$clientname. " not found");
            }
    }


    public function get_current_client_backups(){
    if ($this->session){
        $order=array("sa" => "backups", "clientid" => $this->current_client_id);
        $result=$this->postInformation("backups",$order);
        return($result);
        }else{
            $this->debug("CRITICAL","user ".$clientname. " not found");
        }
    }

    public function get_server_identity(){
    if ($this->session){
      $order=array("");
      $result=$this->postInformation("status",$order);
      return($result->server_identity);
    }else{
      return(-1);
    }
  }


}

?>



