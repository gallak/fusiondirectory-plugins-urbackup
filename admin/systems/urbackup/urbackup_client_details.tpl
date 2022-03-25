<div id="{$sectionId}" class="plugin-section">
  <span class="legend">
    {if $sectionIcon}<img src="{$sectionIcon|escape}" alt=""/>{/if}{$section|escape}
  </span>
    <div>
        {t}Client informations{/t}
    <div>
        {foreach $attributes.client_details as $value}
                {$value}
        {/foreach}

    </div>
</div>
<div>
    {t}Client space consumption{/t}
    <div class="legend">
        {foreach $attributes.client_consumption as $value}
            {$value}
        {/foreach}
    </div>
</div>
</div>
