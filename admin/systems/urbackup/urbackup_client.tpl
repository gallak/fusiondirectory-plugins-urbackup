<div id="{$sectionId}" class="plugin-section">
  <span class="legend">
    {if $sectionIcon}<img src="{$sectionIcon|escape}" alt=""/>{/if}{$section|escape}
  </span>
  <div>
    {$attributes.details_client_software}
    {$attributes.details_client_activity}
    {$attributes.details_client_status}
    {$attributes.details_client_backup_resume}
  </div>

</div>
