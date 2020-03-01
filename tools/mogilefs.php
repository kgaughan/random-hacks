<?php
/** Base for all exceptions thrown by the library. */
class MogileFS_Exception extends Exception {}
/** Base for all exceptions thrown due to tracker error responses. */
class MogileFS_TrackerException extends MogileFS_Exception {}
/** There was a problem with the underlying tracker socket transport. */
class MogileFS_TransportException extends MogileFS_TrackerException {}
/** The given key isn't known to the tracker queried. */
class MogileFS_UnknownKeyException extends MogileFS_TrackerException {}
class MogileFS_NoMatchException extends MogileFS_TrackerException {}
class MogileFS_EmptyFileException extends MogileFS_TrackerException {}
class MogileFS_StoreException extends MogileFS_Exception {}

/**
 * A low-level client encapsulating communication with a set of MogileFS
 * trackers.
 */
class MogileFS_TrackerClient {

	const DEFAULT_PORT = 7001;

	private $conn;
	private $domain;
	private $class;
	private $trackers;
	private $timeout;

	public function __construct($domain, $class, array $trackers, $timeout=2) {
		$this->conn = false;
		$this->domain = $domain;
		$this->class = $class;
		$this->trackers = $trackers;
		$this->timeout = max(0, $timeout);
	}

	public function is_valid() {
		return is_resource($this->conn);
	}

	private function connect() {
		if (!$this->is_valid()) {
			foreach ($this->trackers as $tracker) {
				$this->conn = @stream_socket_client($tracker, $errno, $errstr, $this->timeout);
				if ($this->is_valid()) {
					stream_set_timeout($this->conn, $this->timeout);
					return;
				}
			}
		}
		throw new MogileFS_TransportException("Connection failed");
	}

	public function close() {
		if ($this->is_valid()) {
			fclose($this->conn);
			$this->conn = false;
		}
	}

	private function send($cmd, array $args) {
		$this->connect();
		$args['domain'] = $this->domain;
		$args['class'] = $this->class;
		if (fwrite($this->conn, $cmd . '&' . http_build_query($args) . "\n") === false) {
			$this->close();
			throw new MogileFS_TransportException('Write failed');
		}
		$response = fgets($this->conn);
		if ($response === false) {
			$this->close();
			throw new MogileFS_TransportException('Read failed');
		}
		$parts = explode(' ', $response, 2);
		if (count($parts) == 1) {
			$parts[1] = null;
		} elseif ($parts[0] == 'OK') {
			parse_str($parts[1], $parsed);
			$parts[1] = $parsed;
		}
		return $parts;
	}

	public function call($cmd, array $args) {
		list($code, $response) = $this->send($cmd, $args);
		if ($code == 'OK') {
			return $response;
		}
		switch ($response) {
		case 'empty_file':
			throw new MogileFS_EmptyFileException($args['key']);
		case 'no_match':
			throw new MogileFS_NoMatchException($args['key']);
		case 'unknown_key':
			throw new MogileFS_UnknownKeyException($args['key']);
		default:
			throw new MogileFS_TrackerException(urldecode($response));
		}
	}
}

/**
 * A low-level client encapsulating communication with the HTTP-based
 * filestores of a set of MogileFS trackers.
 */
class MogileFS_StoreClient {

	private $timeout;
	private $ch = false;

	public function __construct($timeout=4) {
		$this->timeout = max(0, $timeout);
	}

	private function connect() {
		if (!is_resource($this->ch)) {
			$this->ch = curl_init();
			curl_setopt($this->ch, CURLOPT_TIMEOUT, $this->timeout);
			curl_setopt($this->ch, CURLOPT_FAILONERROR, true);
			curl_setopt($this->ch, CURLOPT_RETURNTRANSFER, true);
		}
	}

	private function execute(array $opts) {
		$this->connect();
		foreach ($opts as $k => $v) {
			curl_setopt($this->ch, $k, $v);
		}
		$response = curl_exec($this->ch);
		if ($response === false) {
			$error = curl_error($this->ch);
			$this->close();
			throw new MogileFS_StoreException($error);
		}
		return $response;
	}

	public function close() {
		if (is_resource($this->ch)) {
			curl_close($this->ch);
			$this->ch = false;
		}
	}

	public function get($url) {
		return $this->execute(array(
			CURLOPT_HTTPGET => true,
			CURLOPT_URL => $url));
	}

	public function put($url, $fh, $length) {
		if (!is_resource($fh)) {
			throw new MogileFS_StoreException("No file to PUT");
		}
		return $this->execute(array(
			CURLOPT_PUT => true,
			CURLOPT_URL => $url,
			CURLOPT_INFILE => $fh,
			CURLOPT_INFILESIZE => $length));
	}
}

/**
 * A high-level client for communcating with a set of MogileFS trackers and
 * their associated HTTP-based filestores.
 */
class MogileFS_Client {

	private $tracker;
	private $store;

	public function __construct(MogileFS_TrackerClient $tracker, MogileFS_StoreClient $store) {
		$this->tracker = $tracker;
		$this->store = $store;
	}

	public function close() {
		$this->tracker->close();
		$this->store->close();
	}

	public function get_domains() {
		$response = $this->tracker->call('GET_DOMAINS', array());
		$domains = array();
		$n_domains = $response['domains'];
		for ($i = 1; $i <= $n_domains; $i++) {
			$domain_key = 'domain' . $i;
			$classes = array();
			$n_classes = $response[$domain_key . 'classes'];
			for ($j = 1; $j <= $n_classes; $j++) {
				$class_key = $domain_key . 'class' . $j;
				$classes[$response[$class_key . 'name']] = $res[$class_key . 'mindevcount'];
			}
			$domains[] = array('name' => $response[$domain_key], 'classes' => $classes);
		}
		return $domains;
	}

	public function get_paths($key) {
		$response = $this->tracker->call('GET_PATHS', compact('key'));
		unset($response['paths']);
		return $response;
	}

	public function delete($key) {
		$this->tracker->call('DELETE', compact('key'));
	}

	public function rename($from_key, $to_key) {
		$this->tracker->call('RENAME', compact('from_key', 'to_key'));
	}

	public function list_keys($prefix=null, $after=null, $limit=null) {
		return $this->tracker->call('LIST_KEYS', compact('prefix', 'after', 'limit'));
	}

	public function get($key) {
		foreach ($this->get_paths($key) as $url) {
			$response = $this->store->get($url);
			if ($response !== false) {
				return $response;
			}
		}
		return false;
	}

	public function put($key, $fh, $length) {
		$response = $this->tracker->call('CREATE_OPEN', compact('key'));
		$this->store->put($response['path'], $fh, $length);
		$response['key'] = $key;
		$this->tracker->call('CREATE_CLOSE', $response);
	}

	public function put_string($key, $val) {
		$fh = fopen('php://memory', 'rw');
		fwrite($fh, $val);
		rewind($fh);
		try {
			$this->put($key, $fh, strlen($val));
		} catch (Exception $e) {
			fclose($fh);
			throw $e;
		}
		fclose($fh);
	}

	public function put_file($key, $filename) {
		$fh = fopen($filename, 'r');
		if ($fh === false) {
			throw new MogileFS_StoreException("Cannot open '$filename'");
		}
		try {
			$this->put($key, $fh, filesize($filename));
		} catch (Exception $e) {
			fclose($fh);
			throw $e;
		}
		fclose($fh);
	}
}
