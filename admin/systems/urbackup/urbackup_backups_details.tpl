<div id="{$sectionId}" class="plugin-section">
  <span class="legend">
    {if $sectionIcon}<img src="{$sectionIcon|escape}" alt=""/>{/if}{$section|escape}
  </span>
  <div>
        {foreach $attributes.details_backups as $value}
                {$value}
        {/foreach}
  </div>
</div>
