<div id="{$sectionId}" class="plugin-section">
  <span class="legend">
    {if $sectionIcon}<img src="{$sectionIcon|escape}" alt=""/>{/if}{$section|escape}
  </span>
  <div>
    {$attributes.details_image_backups}
  </div>
  <div>
    {$attributes.details_file_backups}
  </div>


</div>
