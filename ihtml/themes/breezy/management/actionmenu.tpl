{function menu level=2}
  <ul class="level{$level}">
  {foreach $data as $entry}
    {if isset($entry.actions)}
      <li id="actionmenu_{$entry.name|escape}">
        <a href="#">
          <img src="{$entry.icon|escape}" alt="{$entry.name|escape}"/>&nbsp;{$entry.label|escape}
          &nbsp;<img src="images/forward-arrow.png" alt="forward arrow"/>
        </a>
        {menu data=$entry.actions level=$level+1}
      </li>
    {else}
      <li id="actionmenu_{$entry.name|escape}">
        <a href="#" onClick="document.getElementById('actionmenu').value='{$entry.name|escape}';document.getElementById('exec_act').click();">
          <img src="{$entry.icon|escape}" alt="{$entry.name|escape}">&nbsp;{$entry.label|escape}
        </a>
      </li>
    {/if}
  {/foreach}
  </ul>
{/function}

<div id="pulldown">
  <input type="hidden" name="act" id="actionmenu" value=""/>
  <div style="display:none"><input type="submit" name="exec_act" id="exec_act" value=""/></div>
  <ul class="level1" id="root">
    <li>
      <a href="#">{t}Actions{/t}&nbsp;<img class="center optional" src="images/down-arrow.png" alt="down arrow"/></a>
      {menu data=$actions}
    </li>
  </ul>
</div>
