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

class urbackup_server{
  private $server_basic_username = "";
  private $server_basic_password = "";
  private $session="";
  private $logged_in = false;
  private $lastlogid = 0;

  // constructueur
  public function __construct($server, $username, $password){
    $this->server_url=$server;
    $this->server_username=$username;
    $this->server_password=$password;
  }

  // get_response
  private function get_response($action, $params=[], $method="POST"){
    $headers = array("Accept: application/json","Content-Type : application/json; charset=UTF-8");
    /* 
     * TODO
     * if('server_basic_username' in globals() and len(self.server_basic_username)>0):
     *    userAndPass = b64encode(str.encode(self.server_basic_username+":"+self.server_basic_password)).decode("ascii")
     *    headers['Authorization'] = 'Basic %s' %  userAndPass
    */

    $actionQuery=array("a" => $action);
    $curr_server_url=$this->server_url."?".http_build_query($actionQuery);
    if(strlen($this->session) > 0){
      $params=array_merge($params,array("ses" => $this->session));
    }


    $h = curl_init();
    $http_timeout = 10*60;
    curl_setopt($h,CURLOPT_URL,$curr_server_url);
    curl_setopt($h,CURLOPT_TIMEOUT,$http_timeout);
    curl_setopt($h,CURLOPT_HTTPHEADER,$headers);
    curl_setopt($h,CURLOPT_RETURNTRANSFER,true);


    if ( ! $method ){
      $method = "POST";
    }

    if ( $method == "GET" ){
      $curr_server_url=$curr_server_url."&".http_build_query($params);
    }

    $target = parse_url($curr_server_url);

    if ($method == "POST"){
      curl_setopt($h,CURLOPT_HEADER, false);
      curl_setopt($h,CURLOPT_POSTFIELDS,http_build_query($params));
    }else{
      $body = "";
    }

//    $this->debug("-------METHODE : $method --------\n");
//    $this->debug(var_dump($params)."\n");
//    $this->debug(var_dump($curr_server_url."\n"));

    $hResponse = curl_exec($h);

    if(curl_getinfo($h,CURLINFO_HTTP_CODE) == 200){
      return $hResponse;
    }else{
      //$this->debug("API call failed. ...");
      return(-1);
    }
  }

  /*
    private function get_json($action, $params = "{}"){
    $tries = 50;
        while ( $tries > 0 ){

            $response = $this->get_response($action, $params);

        

            //if(curl_getinfo($s,CURLINFO_HTTP_CODE) == 200){

            //    break;

      //}

            

            $tries = $tries - 1;

            if ( $tries==0 ){

                return(-1);

            }else{

                $this->debug("API call failed. Retrying...");

            }

        

        $data = $response;

        return json_decode(utf8_decode($data));

    }

  } */

  /*

    private _download_file(action, outputfn, params){

  }

  

    private _md5(s){

  }*/

  

  private function debug($msg){
    print($msg);
  }

  public function login(){
    if ( ! $this->logged_in) {
//      $this->debug("Trying anonymous login...\n");
//      $login = $this->get_response("login", null);
//      $this->debug("Response anonumous login : ".(string)$login."\n");
//      var_dump($login);
//      $this->debug("---------------\n");         

//      if (empty($login) or json_decode($login)->success == false ){
//        $this->debug("Try Logging in...no anonymous login");
        $user=array('username' => $this->server_username );
        $salt = json_decode(utf8_encode($this->get_response("salt", $user)));
        if( empty($salt) or empty($salt->salt)) {
          return(
		array('resultat' => false, 'msg' => 'Username does not exist', 'obj' => '')
		);
        }

        $this->session = $salt->ses;

        if( isset($salt->salt)){
          $password_md5_bin = md5($salt->salt.$this->server_password,true);
          $password_md5 = utf8_decode(md5($salt->salt.$this->server_password));
          if( $salt->pbkdf2_rounds){
            $pbkdf2_rounds = $salt->pbkdf2_rounds;
            if ($pbkdf2_rounds > 0) {
              $password_md5 = hash_pbkdf2('sha256', $password_md5_bin, $salt->salt,$pbkdf2_rounds);
            }
          }

          $password_md5 = md5($salt->rnd.$password_md5);
          $accreditation=array('username' => $this->server_username, 'password' => $password_md5);
          $login = $this->get_response("login", $accreditation);
          // $login contient la capacite de la personnes connecte
          //var_dump($login);
          $obj = $login;
          if(! isset($login) or json_decode($login)->success == false){
          return(
                array('resultat' => false, 'msg' => 'Error during login. Password wrong?', 'obj' => $obj)
                );
          }else{
            $this->logged_in=True;
            return(
                array('resultat' => true, 'msg' => 'Connexion successful', 'obj' => $obj)
                );
          }
        }else{
            return(
                array('resultat' => false, 'msg' => 'Salt per user not found', 'obj' => $obj)
                );

//          return(false);
        }
      //}else{
       // $this->logged_in=True;
      //  $this->debug("anonymous login OK");
      //  return(true);
      //}
    }else{
      // utilisateur deja connecte
            return(
                array('resultat' => true, 'msg' => 'User already connect', 'obj' => $obj)
                );

    }
  }




    public function get_client_status($clientname){
      if ($this->logged_in){
      $status = $this->get_response("status", array());
      $j= json_decode(utf8_encode($status));
      if (isset($clientname)){
        foreach($j->status as $client){
          if ( $client->name == $clientname ){
            return(array('resultat' => true, 'msg' => "Client Found", 'obj' => $client));
          }
        }
      }else{
        return(array('resultat' => false, 'msg' => "Client not Found", 'obj' => $status));
      }
    }else{
      return(array('resultat' => false, 'msg' => "Error while fetchong client", 'obj' => ''));
    }
  }

  /*

    public function download_installer(installer_fn, new_clientname, e_installer_os){

  }

  

    public function add_client(clientname){

  }

  

    public function get_global_settings(self){

  }

  

    public function set_global_setting(key, new_value){

  }

  */

  public function get_client_settings($clientname){
    if ($this->logged_in){
    //$client=$this->get_client_status($clientname);
      $request=array("sa"=>"clientsettings", "t_clientid" => $this->get_client_id_from_name($clientname));
      $settings=$this->get_response("settings",$request);
      return(json_decode(utf8_encode($settings)));
    }else{
      return(-1);
    }
  }

  public function get_client_id_from_name($clientname){
    if ($this->logged_in){
      $res=$this->get_client_status($clientname);
      if ($res['resultat'] == true ){
        return(array('resultat' => true, 'msg' => "Client Found", 'obj' => $res['obj']->id));
      }else{
      	return(array('resultat' => false, 'msg' => "Unable to get client status", 'obj' => ''));
      }
    }else{
      return(array('resultat' => false, 'msg' => "user not connectet", 'obj' => ''));
    }
  }


  /*

    public function change_client_setting(clientname, key, new_value){

  }

  

    public function get_client_authkey(clientname){

  }

  */
  public function get_status(){
    if ($this->logged_in){
      $order=array("");
      $result=$this->get_response("status",$order);
      return(json_decode(utf8_encode($result))->status);
    }else{
      return(-1);
    }
  }

  public function get_server_identity(){
    if ($this->logged_in){
      $order=array("");
      $result=$this->get_response("status",$order);
      return(json_decode(utf8_encode($result))->server_identity);
    }else{
      return(-1);
    }
  }

  public function get_progress(){
    if ($this->logged_in){
      //$order=array("");
      $result=$this->get_response("progress");
      return(json_decode(utf8_encode($result))->progress);
    }else{
      return(-1);
    }
  }


  public function get_progress_per_id($id){
    if ($this->logged_in){
      //$order=array("");
      $result=$this->get_response("progress");
      $all_jobs = json_decode(utf8_encode($result))->progress;
      $job = array("resultat" => false, "msg"=> "no backup", "obj" => []);
      for ($i=0; $i < count($all_jobs); $i++){
        if( $all_jobs[$i]->clientid == $id )
          $job = array("resultat" => true, "msg"=> "Backup Fount ", "obj" => $all_jobs[$i]);
      }
      return($job);
    }else{
      return(-1);
    }
  }

  

  public function get_users(){
    if ($this->logged_in){
      $order=array("sa" => "listusers");
      $result=$this->get_response("settings",$order);
      return(json_decode(utf8_encode($result)));
    }else{
      return(-1);
    }
  }

  public function get_livelog($clientid){
    if ($this->logged_in){
      $order=array("clientid" => $clientid, "lastid" => $this->lastlogid);
      $settings=$this->get_response("usage",array());
      return(json_decode(utf8_encode($settings)));
    }else{
      return(-1);
    }
  }


  public function get_usage(){
    if ($this->logged_in){
      $settings=$this->get_response("usage",array());
      var_dump($settings);
      return(json_decode(utf8_encode($settings)));
    }else{
      return(-1);
    }
  }



  public function start_backup($clientname,$type){
    $order = array("start_client" => $this->get_client_id_from_name($clientname)['obj'], "start_type" => $type);
    $r=$this->get_response("start_backup",$order);
    return(json_decode(utf8_encode($r)));
  }


  /*

    public function get_extra_clients(self){

  }

  

    public function start_backup(clientname, backup_type){

  }

  */

  public function start_incr_file_backup($client){
    return(json_decode(utf8_encode($result = $this->start_backup($client,"incr_file"))));
  }

  

  public function start_full_file_backup($client){
    $this->start_backup($client,"full_file");
  }

  

  public function start_incr_image_backup($client){
    $this->start_backup($client,"incr_image");
  }

  

  public function start_full_image_backup($client){
    $this->start_backup($client,"full_image");
  }



/*
    def get_clientimagebackups(self, clientid = 0):
        if not self.login():
            return None
        
        backups = self._get_json("backups", { "sa": "backups", "clientid": clientid })
        
        return backups["backup_images"]
    
    def get_clientbackups(self, clientid = 0):
        if not self.login():
            return None
        
        backups = self._get_json("backups", { "sa": "backups", "clientid": clientid })
        
        return backups["backups"]
*/


    public function get_client_backups($clientid = 0 ){
    if ($this->logged_in){
      $order=array("sa" => "backups", "clientid" => $clientid);
      $result=$this->get_response("backups",$order);
      if ($result){
	return(array('resultat' => true, 'msg' => "some backups found", 'obj' => $result));
      }else{
	return(array('resultat' => false, 'msg' => "unable to fetch list of backup", 'obj' => ''));
      }
    }else{
	return(array('resultat' => false, 'msg' => "user not connected", 'obj' => ''));
    }

  }

//return(array('resultat' => false, 'msg' => "user not connectet", 'obj' => ''));

 

    public function get_clientimagebackups($clientid = 0 ){
    if ($this->logged_in){
      $order=array("sa" => "backups", "clientid" => $clientid);
      $result=$this->get_response("backups",$order);
      return(json_decode(utf8_encode($result)));
    }else{
      return(-1);
    }

  }

  

    public function get_clientbackups($clientid = 0){

  }

/*

    public function get_groups(){

  }

  

    public function get_clients_with_group(self){

  }

  

    public function add_extra_client(addr){

  }

  

    public function remove_extra_client(ecid){

  }

  

    public function get_actions(self){

  }

  

    public function stop_action(action){

  }

  */

}

?>


