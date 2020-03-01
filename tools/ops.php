<?php
include 'Projects/afk/fwk/classes/AFK/ElementNode.php';
include 'Projects/afk/fwk/classes/AFK/XmlParser.php';

class OPS_Parser extends AFK_XmlParser {

	private $stack = array();
	private $last_key = null;
	private $in_item = false;

	protected function on_start_tag($tag, array $attrs) {
		if ($tag == 'data_block') {
			// Reset.
			$this->stack = array();
			$this->last_key = null;
			$this->in_item = false;
		} elseif ($tag == 'dt_assoc' || $tag == 'dt_array') {
			// Shove a new array onto the stack, and tie it back its
			// containing one.
			$head = array();
			if (count($this->stack) > 0) {
				$this->stack[count($this->stack) - 1][$this->last_key] =& $head;
			}
			$this->stack[] =& $head;
			$this->in_item = false;
		} elseif ($tag == 'item') {
			// An array item.
			$this->last_key = $attrs['key'];
			$this->in_item = true;
		}
	}

	protected function on_end_tag($tag) {
		$this->in_item = false;
		if (($tag == 'dt_assoc' || $tag == 'dt_array') && count($this->stack) > 1) {
			array_pop($this->stack);
		}
	}

	protected function on_text($text) {
		if ($this->in_item) {
			$this->stack[count($this->stack) - 1][$this->last_key] = trim($text);
		}
	}

	public function get_result() {
		if (count($this->stack) > 0) {
			return $this->stack[0];
		}
		return false;
	}

	public static function serialise(array $arr) {
		$root = new AFK_ElementNode('OPS_envelope');
		$root->header()->version('1.0');
		self::serialise_rec($arr, $root->body()->data_block());
		return str_replace(
			array('<OPS_envelope>', "><"),
			array('<!DOCTYPE OPS_envelope SYSTEM "ops.dtd"><OPS_envelope>', ">\n<"),
			$root->as_xml());
	}

	private static function serialise_rec($val, AFK_ElementNode $wrapper) {
		if (is_array($val)) {
			$all_numeric = true;
			foreach (array_keys($val) as $k) {
				if (!is_numeric($k)) {
					$all_numeric = false;
					break;
				}
			}
			$container = $wrapper->child($all_numeric ? 'dt_array' : 'dt_assoc');
			foreach ($val as $k => $v) {
				$item = $container->item()->attr('key', $k);
				self::serialise_rec($v, $item);
			}
		} else {
			$wrapper->with_text($val);
		}
	}
}

function test() {
	$expected = array(
		'protocol' => 'XCP',
		'object' => 'domain',
		'action' => 'lookup',
		'attributes' => array(
			'domain' => 'MyDomainToLookup.com',
			'nameservers' => array('ns1', 'ns2')));

	$parser = new OPS_Parser();
	$parser->parse(OPS_Parser::serialise($expected));
	$actual = $parser->get_result();
	unset($parser);

	if ($expected == $actual) {
		echo "Hurray!\n";
	} else {
		echo "D'oh!\n";
		print_r(compact('expected', 'actual'));
	}
	echo OPS_Parser::serialise($expected);
}
