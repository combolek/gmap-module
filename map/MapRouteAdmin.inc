<?php
/*
 * Gallery - a web based photo album viewer and editor
 * Copyright (C) 2000-2007 Bharat Mediratta
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or (at
 * your option) any later version.
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street - Fifth Floor, Boston, MA  02110-1301, USA.
 */
/**
 * @package map
 * @author Eric H Daniel <ehdaniel@gmail.com>
 * @version $Revision: 1253 $
 *
 */

GalleryCoreApi::requireOnce('modules/map/classes/mapHelper.class');
GalleryCoreApi::requireOnce('modules/map/classes/GoogleMapUtilities.class');

/* Note: The value of this string must be lower case */
define('IEXMLSTRING', 'xmlns="http://www.w3.org/1999/xhtml" xmlns:v="urn:schemas-microsoft-com:vml"');

/**
 * @package map
 * @subpackage UserInterface
 */
class MapRouteAdminController extends GalleryController {
    /**
     * @see GalleryController::handleRequest
     */
    function handleRequest($form) {
        
	include(dirname(__FILE__) . '/includes/MapAdminControllerInit.inc');

	if (isset($form['action']['createroute'])) {

	    GalleryCoreApi::setPluginParameter('module', 'map', 'EditRoute', '');

	    $results['status'] = $status;
	    $results['error'] = array();
	    $results['redirect']['view'] = 'core.SiteAdmin';
	    $results['redirect']['subView'] = 'map.MapRouteAdmin';
	    $results['redirect']['mode'] = 'AddRoute';

	}

	if (isset($form['action']['deleteallroutes'])) {

	    GalleryCoreApi::setPluginParameter('module', 'map', 'Routes', '');

	    $results['status'] = $status;
	    $results['error'] = array();
	    $results['redirect']['view'] = 'core.SiteAdmin';
	    $results['redirect']['subView'] = 'map.MapRouteAdmin';

	}
	if (isset($form['action']['editroute'])) {

	    $id = substr($form['action']['editroute'], 11);

	    $tmp = GalleryCoreApi::getPluginParameter('module', 'map', 'Routes');
	    $currentroutes = explode('|', $tmp[1]);

	    $newroutes = '';
	    foreach ($currentroutes as $route) {
		$temp = explode('`', $route);
		if ($temp[0] <> $id) {
		    if ($newroutes == '') {
			$newroutes = $route;
		    } else {
			$newroutes = $newroutes . '|' . $route;
		    }
		} else {
		    $editroute = $route;
		}
	    }

	    GalleryCoreApi::setPluginParameter('module', 'map', 'Routes', $newroutes);
	    GalleryCoreApi::setPluginParameter('module', 'map', 'EditRoute', $editroute);

	    $results['status'] = $status;
	    $results['error'] = array();
	    $results['redirect']['view'] = 'core.SiteAdmin';
	    $results['redirect']['subView'] = 'map.MapRouteAdmin';
	    $results['redirect']['mode'] = 'AddRoute';

	}
	if (isset($form['action']['deleteroute'])) {

	    $id = substr($form['action']['deleteroute'], 13);

	    $tmp = GalleryCoreApi::getPluginParameter('module', 'map', 'Routes');
	    $currentroutes = explode('|', $tmp[1]);

	    $newroutes = '';

	    foreach ($currentroutes as $route) {
		$temp = explode('`', $route);
		if ($temp[0] <> $id) {
		    if ($newroutes == '') {
			$newroutes = $route;
		    } else {
			$newroutes = $newroutes . '|' . $route;
		    }
		}
	    }

	    GalleryCoreApi::setPluginParameter('module', 'map', 'Routes', $newroutes);

	    $status['routedeleted'] = 1;

	    $results['status'] = $status;
	    $results['error'] = array();
	    $results['redirect']['view'] = 'core.SiteAdmin';
	    $results['redirect']['subView'] = 'map.MapRouteAdmin';

	}

	if (isset($form['action']['tplupdate'])) {
	    $DefaultTheme = @GalleryCoreApi::getPluginParameter('module', 'core', 'default.theme');

	    $filename = $basegallerydir . '../themes/' . $DefaultTheme[1] . '/templates/theme.tpl';
	    $backup = $basegallerydir . '../themes/' . $DefaultTheme[1] . '/templates/theme.back';
	    if (!file_exists($backup)) {
		copy($filename, $backup);
	    }

	    $file = @file($backup);
	    $handle = @fopen($filename, 'w');

	    if ($file && $handle) {
		foreach($file as $line) {
		    /* Use the triple === to make sure it's not the boolean value false */
		    if (strpos(strtolower(trim($line)), '<html') === 0) {
			/* It doesn't have the magic string */
			if (strpos(strtolower($line), IEXMLSTRING) === false) {
			    $newline = preg_replace('/>$/', ' ' . IEXMLSTRING . ">\n", trim($line));
			    fwrite($handle, $newline);
			} else {
			    fwrite($handle, $line);
			}
		    } else {
			fwrite($handle, $line);
		    }
		}
	    } else {
		$error[] = 'form[error][canotopentemplate]';
	    }
	    $status['changesucess'] = 1;
            
	    $direction = (empty($error)) ? 'redirect' : 'delegate';
	    $results['status'] = $status;
	    $results['error'] = $error;
	    $results[$direction]['view'] = 'core.SiteAdmin';
	    $results[$direction]['subView'] = 'map.MapRouteAdmin';

	}
	if (isset($form['action']['save'])) {
	    /* Some error checking */
	    if (empty($_POST['RouteName'])) {
		$error[] = 'form[error][routename]';
	    } else if (!(empty($_POST['RouteColor']))
		    and (!preg_match('/^#[0-9,A-F]{6}$/', $_POST['RouteColor']))) {
		$error[] = 'form[error][routecolor]';
	    } else if (!(empty($_POST['RouteOpacity'])) 
		    and (($_POST['RouteOpacity'] > 1) || ($_POST['RouteOpacity'] < 0))) {
		$error[] = 'form[error][routeopacity]';
	    } else if ((count($_POST) - 13) < 2) {
		$error[] = 'form[error][toofewarg]';
	    } else {

	    	$RouteColor = $_POST['RouteColor'];
	    	if (empty($RouteColor)) {
		    $RouteColor = '#8A2BE2';
	    	}
		$RouteName = $_POST['RouteName'];
		$RouteWeight = $_POST['RouteWeight'];
		if (empty($RouteWeight)) {
		    $RouteWeight = '3';
		}
		$RouteOpacity = $_POST['RouteOpacity'];
		if (empty($RouteOpacity)) {
		    $RouteOpacity = '1';
		}
		$RouteEnabled = $_POST['RouteEnabled'];
		$RouteFilter = $_POST['RouteFilter'];

		$lastkey = '';
		$lastvalue = '';
		$hash = array();

		foreach ($_POST as $key => $value) {

		    if ($key != 'g2_returnName' and $key != 'g2_navId' and $key != 'g2_formUrl' 
			    and $key != 'g2_controller' and $key != 'g2_form') {
			if ($key == 'Order' . $lastkey) {
			    $commapos = strpos($value, ',');
			    if (!is_numeric($commapos) && !$commapos) {
				$hash[$value] = '`' . $lastvalue;
			    } else {
				$allvalues = explode(',', $value);
				foreach ($allvalues as $values) {
				    $hash[$values] = '`' . $lastvalue;
				}
			    }
			}
			$lastkey = $key;
			$lastvalue = $value;
		    }
		}
		ksort($hash, SORT_NUMERIC);
		$allgpspoint = '';
		foreach ($hash as $point) {
		    $allgpspoint = $allgpspoint . $point;
		}

		$oldroutes = GalleryCoreApi::getPluginParameter('module', 'map', 'Routes');

		if ($oldroutes[1] != '') {
		    /* Get the Route ID if editing an existing one, otherwise initialize to 0 */
		    $num = isset($_POST['oldid']) ? $_POST['oldid'] : 0;

		    /* Make sure all the routes have unique IDs */
		    $routestrings = explode('|', $oldroutes[1]);
		    foreach($routestrings as $temproute) {
		    	$temproutes[] = explode('`', $temproute);
		    }
		    $maxRouteId = 0;
		    $editIdDupe = false;
		    $routesById = array();
		    foreach($temproutes as $routeKey => $routeArray) {
			$maxRouteId = max(array($maxRouteId, $routeArray[0]));
			if (array_key_exists($routeArray[0], $routesById)) { /* Duplicate ID */
			    /* Check if the ID being edited is a duplicate */
			    if ($num == $routeArray[0]) {
				$editIdDupe = true;
			    }
			    /* Move the dupe to the end */
			    $maxRouteId = $maxRouteId + 1;
			    /* reassign route ID to highest ID + 1 */
			    $temproutes[$routeKey][0] = $maxRouteId;
			}
			/* Store the route string into the array, keyed by route ID */
			$routesById[$temproutes[$routeKey][0]] = 
				implode('`', $temproutes[$routeKey]);
		    }
		    /*
		     * if $num was not found in the existing Route IDs (i.e., it's new) 
		     * or if it was a dupe, then add it as a new route, else leave it alone
		     */
		    $num = ($num == 0 or $editIdDupe) ? $maxRouteId + 1 : $num;
		} else {
		    $num = 1;
		}

		$routesById[$num] = $num . '`'
				    . $RouteName . '`'
				    . $RouteColor . '`'
				    . $RouteWeight . '`'
				    . $RouteOpacity . '`'
				    . $RouteEnabled . '`'
				    . $RouteFilter
				    . $allgpspoint;
		$form['Routes'] = implode('|', $routesById);

		GalleryCoreApi::setPluginParameter('module', 'map', 'Routes', $form['Routes']);
	    }

	    if (empty($error)) {

		$status['routesaved'] = 1;

		GalleryCoreApi::setPluginParameter('module', 'map', 'EditRoute', '');

		$results['status'] = $status;
		$results['error'] = $error;
		$results['redirect']['view'] = 'core.SiteAdmin';
		$results['redirect']['subView'] = 'map.MapRouteAdmin';
	    } else {
		$results['status'] = $status;
		$results['error'] = $error;
		$results['delegate']['view'] = 'core.SiteAdmin';
		$results['delegate']['subView'] = 'map.MapRouteAdmin';
		$results['delegate']['mode'] = 'AddRoute';
	    }
	}

	if (isset($form['action']['cancel'])) {

	    $results['status'] = $status;
	    $results['error'] = array();
	    $results['redirect']['view'] = 'core.SiteAdmin';
	    $results['redirect']['subView'] = 'map.MapRouteAdmin';

	}

	if (isset($form['action']['savesettings'])) {
		
		if (isset($form['EnableRouteNumber']) and $form['EnableRouteNumber']){
			GalleryCoreApi::setPluginParameter('module', 'map', 'EnableRouteNumber', 1);
		} else {
			GalleryCoreApi::setPluginParameter('module', 'map', 'EnableRouteNumber', 0);
		}
		

	    $results['status'] = $status;
	    $results['error'] = array();
	    $results['redirect']['view'] = 'core.SiteAdmin';
	    $results['redirect']['subView'] = 'map.MapRouteAdmin';

	}

	return array (null, $results);
    }
}

/**
 * @package map
 * @subpackage UserInterface
 */
class MapRouteAdminView extends GalleryView {
    /**
     * @see GalleryView::loadTemplate
     */
    function loadTemplate(&$template, &$form) {

	include(dirname(__FILE__) . '/includes/MapAdminViewInit.inc');
	$template->head('modules/map/includes/GoogleMap.css');
	$form['formName'] = 'MapRouteAdmin';

	list ($ret, $dateFormat) =
		GalleryCoreApi::getPluginParameter('module', 'core', 'format.date');

	$form['now'] = strftime($dateFormat, strtotime('now'));

	/* Select items that have GPS coordinates filled out */
	$storage =& $gallery->getStorage();
	$query = '
        SELECT
	  [mapMap::itemId],
	  [mapMap::value]
	FROM
	  [mapMap]
	WHERE
	  [mapMap::field] = \'GPS\'
	';
	list ($ret, $results) = $gallery->search($query, array(), array());

	if (!$ret) {
	    $result1 = $results->nextResult();
	}

	if (!empty($result1)) {
	    list ($ret, $results) = $gallery->search($query, array(), array());

	    /*
	     * Builds a hash for each mappable entity from its ID to a hash of
	     * properties. Not strictly necessary, since it just gets flattened
	     * out later, but it could be useful at some point.
	     */
	    $ids = array();
	    $pointHash = array();
	    while ($result = $results->nextResult()) {
		$id = $result[0];

		$ids[] = $id;
		$pointHash[$id] = array();
		$pointHash[$id]['id'] = $id;
		$pointHash[$id]['gps'] = $result[1];
            
		$tmp = array();
		if (substr($id, 0, 1) == 'G') {
		    if ((array_key_exists('EditGroup', $form) and $form['EditGroup'] != '')
			    and (array_key_exists('mode', $form) and $form['mode'] == 'AddGroup')) {
			$tmp = explode('`', $form['EditGroup']);
			if ($tmp[0] == substr($id, 1)) {
			    unset($pointHash[$id]);
			}
		    }
		    if (array_key_exists($id, $pointHash)) {
			$pointHash[$id]['type'] = 'GoogleMapGroup';
			$groupid = substr($id, 1);
			list ($ret, $tmpparam) =
				GalleryCoreApi::getPluginParameter('module', 'map', 'MapGroups');
			$groups = explode('|', $tmpparam);
			foreach ($groups as $group) {
			    if (!empty($group)) {
				$groupinfo = explode('`', $group);
				if ($groupinfo[0] == $groupid and $groupid != '') {
				    $thumbpic = $basegallerydir . 'map/images/groups/'
						. $groupinfo[4];
				    $pointHash[$id]['title'] = $groupinfo[1];
				    $pointHash[$id]['date'] = strftime($dateFormat, $groupinfo[5]);
				    $pointHash[$id]['description'] = $groupinfo[3];
				    $pointHash[$id]['thumb'] =
					    GalleryUtilities::convertPathToUrl($thumbpic);
				}
			    }
			}
		    }
		}
	    }

	    /* create an ID array without the groups in it */
	    $tmp = array();
	    foreach ($ids as $id) {
		if (substr($id, 0, 1) != 'G') {
		    $tmp[] = $id;
		}
	    }
	    $ids = $tmp;


	    /* Trim the list down to the items that the user is allowed to see */
	    list ($ret, $perms) =
		    GalleryCoreApi::fetchPermissionsForItems($ids, $gallery->getActiveUserId());
	    for($i = 0; $i < count($ids); $i++) {
		if (!array_key_exists($ids[$i], $perms)
			|| !array_key_exists('core.view', $perms[$ids[$i]])) {
		    unset($pointHash[$ids[$i]]);
		    unset($ids[$i]);
		}
	    }

	    $ids = array_values($ids);
	    /* Retrieve data for each entity */
	    list ($ret, $entities) = $storage->loadEntities($ids);
	    list ($ret, $thumbs) = GalleryCoreApi::fetchThumbnailsByItemIds($ids);

	    /*
	     * Fill out additional properties for each entity, taking care to
	     * format them so that they don't break the Javascript that they
	     * end up embedded within.
	     */
	    foreach ($entities as $entity) {
		$pointHash[$entity->getId()]['type'] = $entity->getEntityType();
		$pointHash[$entity->getId()]['title'] =
			preg_replace(array('/\n/', '/\r/'), array('', ''), $entity->getTitle());
		$pointHash[$entity->getId()]['description'] =
			preg_replace(array('/\n/', '/\r/'),
				     array('', ''),
				     $entity->getDescription());
		$pointHash[$entity->getId()]['thumb'] =
			$urlGenerator->generateUrl(
			array('view' => 'core.DownloadItem',
			      'itemId' => $thumbs[$entity->getId()]->getId(),
			      'serialNumber' => $thumbs[$entity->getId()]->getSerialNumber()));
		$pointHash[$entity->getId()]['date'] =
			strftime($dateFormat, $entity->getOriginationTimestamp());
	    }


	    $GPSItem = array();
	    foreach ($pointHash as $id => $point) {
	    	$GPSItem[] = $point;
	    }

	    $form['items'] = $GPSItem;
	}
        
	include(dirname(__FILE__) . '/includes/MapAdminEdits.inc');

	/* Create the route array */
	if (array_key_exists('Routes', $param) and $param['Routes'] != '') {
	    $routes = explode('|', $param['Routes']);
	    natsort($routes);
	    foreach ($routes as $route) {
	    	$form['theroutes'][] = explode('`', $route);
	    }
	}

	/* checking for the right parameters in template file */
	$DefaultTheme = @GalleryCoreApi::getPluginParameter('module', 'core', 'default.theme');
	$filename = $basegallerydir . 'themes/' . $DefaultTheme[1] . '/templates/theme.tpl';
	$file = @file($filename);

	if ($file) {
	    foreach($file as $line) {
		/* Use the triple === to make sure it's not the boolean value false */
		if (strpos(strtolower(trim($line)), '<html') === 0) {
		    /* It doesn't have the magic string */
		    if (strpos(strtolower($line), IEXMLSTRING) === false) {
			$form['badhtmltag'] = 1;
			break;
		    }
		}
	    }
	}
	
	/* Get the parameter for Route Numbering */
	list($ret, $EnableRouteNumber) = @GalleryCoreApi::getPluginParameter('module', 'map', 'EnableRouteNumber');
	if (empty($EnableRouteNumber)) {
		$EnableRouteNumber = 0;	
	}
	
	$form['EnableRouteNumber'] = $EnableRouteNumber;

	if ($template->hasVariable('SiteAdmin')) {
	    $SiteAdmin =& $template->getVariableByReference('SiteAdmin');
	    $SiteAdmin['enctype'] = 'multipart/form-data';
	} else {
	    $SiteAdmin['enctype'] = 'multipart/form-data';
	    $template->setVariable('SiteAdmin', $SiteAdmin);
	}

	$template->setVariable('controller', 'map.MapRouteAdmin');
	return array(null, array('body' => 'modules/map/templates/MapRouteAdmin.tpl'));
    }
}
?>