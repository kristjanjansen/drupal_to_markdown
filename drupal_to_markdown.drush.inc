<?php

/**
 * @file
 *   Drush command to export site contents as Markdown files
 */

/**
 * Implementation of hook_drush_command().
 */
function drupal_to_markdown_drush_command() {
  $items = array();

  $items['drupal-to-markdown'] = array(
    'callback' => 'drupal_to_markdown_export',
    'description' => "Exports site as Markdown files",
    'aliases' => array('dtm'),
    'options' => array(
      'types' => 'Comma-separated node types (default story)',
      'count' => 'Number of nodes to export (default 10)',
      'path' => 'Output directory (default /markdown under files dir)',
    ),
  );

  return $items;
}

/**
 * Drush command callback.
 *
 */
 
function drupal_to_markdown_export() {
  
  global $base_url;
  
  define('HTML2MD_HEADER_STYLE', 'ATX');
  define('HTML2MD_SUPPRESS_ERRORS', false);
  
  require_once DRUSH_BASE_PATH . '/commands/pm/download.pm.inc';
  require_once( dirname( __FILE__) . '/html2markdown/html2markdown.php' ); 
  
  $path = drush_get_option('path') ? drush_get_option('path') : file_directory_path() . '/markdown';
  
  /*
  if (!is_dir($path)) { 
     @drush_op('mkdir', $path);      
   }
  */
  
  $node_types = array('story');
  
  if (drush_get_option('types')) {
    $types = explode(',', drush_get_option('types'));    
    $node_types = (count($types) > 0 ? $types : $node_types);
  }
  
  $limit = drush_get_option('count') ? drush_get_option('count') : 10;
  
  $result = db_query('SELECT nid FROM {node} WHERE type IN ('. db_placeholders($node_types, 'text'). ') ORDER BY created DESC LIMIT 0, ' . $limit, $node_types);

   
  while ($row = db_fetch_object($result)) {
    $node = node_load($row->nid);


    if ($node->taxonomy) {
      $tags = "";
      foreach($node->taxonomy as $tag) {
          if ($tag->name == 'Radio') {
            $tag->name = 'podcast';
          }
          $tags .= "- " . strtolower($tag->name) . "\n";
      }
    }

    $title = ($node->title);
    $title = str_replace(array(':', '"', '?'), '', $title);
      
    $header   = "---\n";
    $header  .= "layout: post\n";
    $header  .= "title: " . $title . "\n";
    $header  .= "permalink: node/" . $node->nid  . "\n";
    $header  .= ($tags ? "tags: \n" . $tags : "");
    $header  .= "type: " . $node->type . "\n";
    $header  .= ($node->status == 0 ? "published: false\n" : "");
    $header  .= "---\n\n";
    
    $body = $node->body;
    
    $body = str_replace('preset=preview','preset=fullsize', $body);
    $body = check_markup($body);
    $body = str_replace($base_url . '/'. file_directory_path(), 'files', $body);
    $body = str_replace('http://kika.trip.ee/sites/kika.trip.ee/files', 'files', $body);
    $body = str_replace('http://kikamele.trip.ee/sites/kika.trip.ee/files', 'files', $body);
    $body = str_replace('"files', '"/files', $body);
        
    $body = str_replace('<pre>', "```\n", $body);
    $body = str_replace('</pre>', "\n```\n\n", $body);
    $body = str_replace(array('“', '”'), '"', $body);
    $body = str_replace('→', '->', $body);

    $body = preg_replace('/\<(.?)(div)(.*)\>/i', '', $body);
         
    $contents = $header . html2markdown($body);
 
    $title = str_replace(' ', '-', $title);
    $title = strtolower($title);
    
//    $file_path = $path . '/' .  format_date($node->created, 'custom', 'Y-m-d') . '-node-' . ($node->nid) . '.md';
    $file_path = $path . '/' .  format_date($node->created, 'custom', 'Y-m-d') . '-node-' . ($node->nid) . ($node->status == 0 ? "-UNPUBLISHED" : "") . '.md';
 
    file_put_contents($file_path , $contents);

  }
 
}



