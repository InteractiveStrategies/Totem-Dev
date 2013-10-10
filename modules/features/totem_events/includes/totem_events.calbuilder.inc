<?php
/**
 * @file
 * totem_events.calbuilder.inc
 */

/**
 * Utility class calblock_builder
 */
class TotemEventsCalendar {

  /**
   * Build an HTML table for a calendar month
   * @param int $year
   * @param int $month
   * @return str HTML
   */
  public function buildCalendar($year, $month) {

    $timestamp = mktime(0, 0, 0, $month, 1, $year);
    $maxday = date("t", $timestamp);
    $thismonth = getdate($timestamp);
    $startday = $thismonth['wday'];
    $today = mktime(0, 0, 0, date('n'), date('j'), date('Y'));

    // Start the html table.
    $html = '<table id="calblock">
      <tr>
        <th class="monthname" colspan="7">
        <h2>' . $thismonth['month'] . ' ' . $thismonth['year'] . '</h2>
        </th>
      </tr>
      <tr>';
    // Day names for headings.
    $days = array('Sun', 'Mon', 'Tue', 'Wed', 'Thu', 'Fri', 'Sat');
    for ($i = 0; $i < 7; $i++) {
      $html .= '<th>' . $days[$i] . '</th>';
    }
    $html .= '</tr>';

    // For each day in the month, figure out what cell to create.
    for ($i = 0; $i < ($maxday + $startday); $i++) {
      // Starting a new week / row.
      if (($i % 7) == 0) {
        $html .= "<tr>\n";
        $daycount = 0;
      }

      if ($i < $startday) {
        // Padding day at beginning of month.
        $html .= "<td> </td>\n";
      }
      else {
        // Normal day.
        $html .= self::formatDayCell(mktime(0, 0, 0, $thismonth['mon'], ($i - $startday + 1), $thismonth['year']), $today);
      }

      $daycount++;

      // End of week / row.
      if (($i % 7) == 6) {
        $html .= "</tr>\n";
      }
    }

    // Padding days at end of month.
    if ($daycount < 7) {
      for ($i = 0; $i < (7 - $daycount); $i++) {
        $html .= '<td> </td>';
      }
      $html .= '</tr>';
    }

    $html .= '</table>';

    return $html;
  }


  /**
   * Format a day cell with relevant background color / link, etc.
   * @param int $stamp Timestamp for date to render
   * @param int $today Timestamp for today's date to compare to
   * @return str HTML
   */
  private function formatDayCell($stamp, $today) {
    $html = '';
    $has_event = self::dayHasEvents($stamp);
    $class = ($has_event) ? 'has_event' : '';
    if ($stamp == $today) {
      $class .= ' today';
    }

    if (!$has_event) {
      $html = '<td class="' . $class . '">' . date('j', $stamp) . '</td>';
    }
    else {
      $html .= sprintf('<td class="%s"><a href="%s">%s</a></td>',
        $class,
        '/events?date=' . date('F-Y', $stamp),
        date('j', $stamp)
      );
    }
    return $html;
  }


  /**
   * Check to see if a date has any events happening that day
   * @param int $stamp Timestamp of day to check
   * @return bool TRUE if there are event(s) that day
   */
  private function dayHasEvents($stamp) {

    $event_days = self::getEventDays();
    return in_array(date('Y-m-d', $stamp), $event_days);
  }


  /**
   * Get an array of dates for the next two months that have events
   * @return array of dates that have events
   */
  private function getEventDays() {

    $event_days = &drupal_static(__FUNCTION__);
    if (!empty($event_days)) {
      return $event_days;
    }

    $query = db_select('node', 'n');
    $query->addExpression('UNIX_TIMESTAMP(field_dates_value)', 'start');
    $query->addExpression('UNIX_TIMESTAMP(field_dates_value2)', 'end');
    $query->join('field_data_field_dates', 'fdfd', 'n.vid = fdfd.revision_id');
    $query->condition('fdfd.bundle', 'event', '=')
      ->condition('n.status', '1', '=');
    $query->where('DATE_FORMAT(field_dates_value, :format_cond1) >= :limit_cond1', array(':format_cond1' => '%m', ':limit_cond1' => date('m')))
      ->where('DATE_FORMAT(field_dates_value, :format_cond2) <= :limit_cond2', array(':format_cond2' => '%m', ':limit_cond2' => str_pad((date('m') + 1), 2, "0", STR_PAD_LEFT)));

    $result = $query->execute();

    $days = array();
    foreach ($result as $res) {
      // Raw db datetimes are UTC.  Must convert to site timezone.
      $start = new DateTime('@' . $res->start);
      $start->setTimezone(new DateTimeZone(date_default_timezone()));
      $start = $start->format('U');
      $end = new DateTime('@' . $res->end);
      $end->setTimezone(new DateTimeZone(date_default_timezone()));
      $end = $end->format('U');
      $days[] = date('Y-m-d', $start);
      $days[] = date('Y-m-d', $end);
      // If there's a span of days between them, include those too.
      if ($end - $start > 86400) {
        while ($end - $start > 86400) {
          $start += 86400;
          $days[] = date('Y-m-d', $start);
        }
      }
    }

    // Even if there are no event days available, we store a value in the array.
    // So drupal_static has something to return.  Otherwise, the static var.
    // Would be empty every time and we'd do all this work for every day.
    $event_days = (empty($days)) ? array(1) : $days;
    return $event_days;
  }

}
